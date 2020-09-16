#include "garbage_collection.hpp"
#include <iostream>
#include <list>
#include <loguru.hpp>
#include "environment.hpp"
#include "scheme.hpp"

namespace scm {

// keep track of how many objects we've created in the lifetime of the program
static int totalObjectCount{0};

// keep track of all existing objects
// std::vector<Collectable*> ObjectHeap;

// constructor and destructor for Collectable class
Collectable::Collectable() : marked(false)
{
  id = totalObjectCount++;
  // keep track of the newly created object
  ObjectHeap.push_back(this);
  DLOG_IF_F(
      INFO, LOG_GARBAGE_COLLECTION, "create Obj:%d (marked: %d)", static_cast<int>(id), marked);
}

Collectable::~Collectable()
{
  DLOG_IF_F(ERROR, LOG_GARBAGE_COLLECTION, "delete Obj:%d", static_cast<int>(id));
}

/**
 * Marks a scheme object as not to be deleted during garbage collection.
 * Will recursively call itself in case of cons objects.
 * @param obj the pointer to an object that's to be marked.
 */
void markSchemeObject(Object* obj)
{
  switch (getTag(obj)) {
    // in most cases, simply mark the object
    case TAG_INT:
    case TAG_FLOAT:
    case TAG_STRING:
    case TAG_SYMBOL:
    case TAG_NIL:
    case TAG_TRUE:
    case TAG_FALSE:
    case TAG_VOID:
    case TAG_EOF:
    case TAG_FUNC_BUILTIN:
    case TAG_SYNTAX:
      obj->marked = true;
      break;
    // user functions consist of two cons objects, the argument and bodylist
    case TAG_FUNC_USER:
      markSchemeObject(getUserFunctionArgList(obj));
      markSchemeObject(getUserFunctionBodyList(obj));
      break;
    // recur until we've reached the end of the list
    case TAG_CONS:
      markSchemeObject(getCar(obj));
      markSchemeObject(getCdr(obj));
      break;

    default:
      schemeThrow("tag " + std::to_string(obj->tag) + " isn't handled yet");
      break;
  }
};

/**
 * Mark all objects reachable from a given environment as not to be collected.
 * @param env the environment from which an object needs to be reachable in order to be accepted
 */
void mark(Environment& env)
{
  for (auto& binding : env.bindings) {
    DLOG_IF_F(INFO,
              LOG_GARBAGE_COLLECTION,
              "marking binding %s | %s",
              binding.first.c_str(),
              toString(binding.second).c_str());
    markSchemeObject(binding.second);
  }
}

/**
 * Delete all objects that weren't marked or aren't essential.
 */
void sweep()
{
  int nObjectsBefore{static_cast<int>(ObjectHeap.size())};
  for (auto i = ObjectHeap.begin(); i != ObjectHeap.end();) {
    if (!(*i)->marked && !(*i)->essential) {
      DLOG_IF_F(INFO,
                LOG_GARBAGE_COLLECTION,
                "delete %s %s",
                tagToString(getTag((Object*)(*i))).c_str(),
                toString((Object*)(*i)).c_str());
      // TODO: this doesn't seem to work yet -> no deleting as of yet :)
      // delete *i;
      i = ObjectHeap.erase(i);
    }
    else {
      DLOG_IF_F(INFO,
                LOG_GARBAGE_COLLECTION,
                "keep %s %s",
                tagToString(((Object*)(*i))->tag).c_str(),
                toString((Object*)(*i)).c_str());
      (*i)->marked = false;
      ++i;
    }
  }
  int nObjectsAfter{static_cast<int>(ObjectHeap.size())};
  DLOG_IF_F(WARNING,
            LOG_GARBAGE_COLLECTION,
            "cleaned up %d/%d objects",
            nObjectsBefore - nObjectsAfter,
            nObjectsBefore);
}

/**
 * Check which objects are still reachable from a given environment and delete the rest.
 * Implementation of a simple mark and sweep algorithm.
 * @param env the environment from which the objects need to be reachable in order not to be
 * deleted.
 */
void markAndSweep(Environment& env)
{
  mark(env);
  sweep();
}

}  // namespace scm