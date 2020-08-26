;; some sugar
(define def define)
(define nil '())

;; constants
(define pi 3.14) ;; who needs those other digits anyway ;)

;; functions -- logical
(define to-bool
    (lambda (x)
        (if x #t #f)))

(define not (lambda (x) (if x #f #t)))

(define and 
    (lambda (x y) 
        (if x
            (to-bool y)
            #f)))

(define or 
    (lambda (x y) 
        (if x
            #t
            (to-bool y))))

(define xor
    (lambda (x y)
        (and
            (not (and x y))
            (or x y))))

(define nand
    (lambda (x y) 
        (not (and x y))))

;; comparisons

(define >
    (lambda (x y)
        (and (not (< x y)) (not (= x y)))))

(define >=
    (lambda (x y)
        (not (< x y))))

(define <=
    (lambda (x y)
        (not (< x y))))

(define max
    (lambda (x y)
        (if (< x y) y x)))

(define min
    (lambda (x y)
        (if (< x y) x y)))

(define pow
    (lambda (num n)
        (if (= n 0)
            1
            (* num (pow num (- n 1))))))

;; higher order functions

(define for-loop (lambda (start stop fn)
    (if (>= start stop)
       nil
    ; else
       (begin
	   (fn start)
	   (for-loop (+ 1 start) stop fn)
       )
    )
))

;; misc

(define sum-below 
    (lambda (n)
        (if (= n 0)
            0
            (+ n (sum-to1 (- n 1))))))
