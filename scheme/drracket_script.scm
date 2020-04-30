#lang scheme

(* (+ 1 2) 3)


(define a 0)
(define b 0)
(define c 0)

(set! a 1)
(set! b (* a 5))


(set! a 10)
;(if (>= a 10)
;    (display "yes")
;    (display "no")
;)

; ----------------------------------------
(define nil '())

(define (myLength l)
    (display "mylength: ")
    (display l)
    (display "\n")
    (if (eq? l nil)
	0
	(+ (myLength (cdr l)) 1)))

(define (make-rev-list n)
    (if (< n 1)
	nil
	(cons n (make-rev-list (- n 1)))))

(define (make-list n)
    (define (make-list-helper start stop)
	(if (> start stop)
	    nil
	    (cons start (make-list-helper (+ 1 start) stop))))
    (make-list-helper 1 n))

(define (myLengthAlt l)
    (define (length-helper lenSoFar restList)
      (if (eq? restList nil)
	  lenSoFar
	  (length-helper (+ 1 lenSoFar) (cdr restList))))
    (length-helper 0 l))

; -------------------------
; peano zahlen

(define _0 nil)
(define (SUCC n) (cons nil n))
(define _1 (SUCC _0))
(define _2 (SUCC _1))
(define _3 (SUCC _2))
(define _4 (SUCC _3))
(define _5 (SUCC _4))
(define _6 (SUCC _5))
(define _7 (SUCC _6))
(define _8 (SUCC _7))
(define _9 (SUCC _8))



(define (PRED n) (cdr n))
(define (_+ a b)
    (if (eq? b _0)
	a
	(_+ (SUCC a) (PRED b))))

(define (_- a b)
    (if (eq? b _0)
	a
	(_- (PRED a) (PRED b))))

(define (_* a b)
  (display "_")
    (if (eq? a _0)
	_0
	(if (eq? b _0)
	    _0
            (if (eq? a _1)
                b
                (if (eq? b _1)
                    a
                    (_+ a (_* a (PRED b))))))))

(define (_/ a b)
  (if (eq? a b)
      _1
      (if (eq? a _0)
          (display "BOOM!")
          (if (eq? b _0)
              (display "BOOM!")
              (display "something else")))))

(_* _3 _4)

(define (_= a b)
  (if (eq? a _0)
      (eq? b _0)
      (if (eq? b _0)
	  #f
	  (_= (PRED a) (PRED b)))))

(define (_> a b)
  (if (eq? b _0)
      (not (eq? a _0))
      (if (eq? a _0)
          #f
          (_> (PRED a) (PRED b)))))


 (define (_< a b)
   (_> b a))

(define (_>= a b)
  (if (_= a b)
      #t
      (_> a b)))

(define (_<= a b)
  (_>= b a))
; ------------------------------------------------------
; objekte vs. closures
;
(define (myCons a b)
  (lambda (getWhat)
    (if (eq? getWhat 0)
	a
	b)))

(define (myCar x)
  (x 0))

(define (myCdr x)
  (x 1))

(define (make-point x y)
  (lambda (what optArg)
    (if (eq? what 0)
	x
	(if (eq? what 1)
	    y
	    (if (eq? what 2)
	       (set! x optArg)
	       (if (eq? what 3)
		   (set! y optArg)
		   (if (eq? what 4)
		       (* x y)
			(error "no such getter"))))))))

(define (point-x p) (p 0 nil))
(define (point-y p) (p 1 nil))
(define (point-set-x p arg) (p 2 arg))
(define (point-set-y p arg) (p 3 arg))
(define (point-area p) (p 4 nil))

(define pnt1 (make-point 110 99))
(point-x pnt1)

(point-y pnt1)

(point-set-x pnt1 99999)
(point-x pnt1)

