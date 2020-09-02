;; some sugar
(define def define)
(define nil '())

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
        (not (> x y))))

(define (equal? x y)
    (if (and (string? x) (string? y))
        (equal-string? x y)
        (if (and (number? x) (number? y))
            (= x y)
            #f)))

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

;; arithmetics
(define (% num denum)
    (define (helper num denum count)
        (if (< num denum)
            num
            (helper (- num denum) denum (+ count 1))))
    (if (= denum 0)
        nil
        (helper num denum 0))
)

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

(define (factorial x)
    (if (= x 0)
        0
        (if (= x 1)
            1
            (* x (factorial (- x 1))))))

(define sum-below 
    (lambda (n)
        (if (= n 0)
            0
            (+ n (sum-below (- n 1))))))

(define fib (lambda (n)
  (if (<= n 2)
      1
      (+ (fib (- n 1)) (fib (- n 2))))))

;; constants
(define pi (/ 22 7)) ;; who needs the real pi anyways ;)