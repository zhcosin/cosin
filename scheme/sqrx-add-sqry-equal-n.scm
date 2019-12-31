
;; 解方程 x^2+y^2=n (x<=y) 的迭代过程
;; 当前取值 x=a,y=b
;; 如果 x^2+y^2=n，那么(x,y)=(a,b)为一解，加入结果l中，并取(x,y)=(a+1,a+1)转下一步迭代
;; 如果 x^2+y^2<n，那么取(x,y)=(a,b+1)，转下一步迭代
;; 如果 x^2+y^2>n，此时如果x^2<n，取(x,y)=(a+1,a+1)转下一步迭代，否则迭代结束，l 就是结果.
(define (solve-sqrx-add-sqry-equal-n-iter n root-n a b l)
  (let ((s (+ (* a a) (* b b))))
    (cond ((= s n) (solve-sqrx-add-sqry-equal-n-iter n root-n (+ a 1) (+ a 1) (cons (cons a b) l)))
          ((< s n) (solve-sqrx-add-sqry-equal-n-iter n root-n a (+ b 1) l))
          (else (if (>= a root-n)
                    l
                    (solve-sqrx-add-sqry-equal-n-iter n root-n (+ a 1) (+ a 1) l))))))   

;; 解方程 x^2+y^2=n (x<=y)，并且限定x 最小为 min
(define (solve-sqrx-add-sqry-equal-n-with-min n min)
  (solve-sqrx-add-sqry-equal-n-iter n (sqrt n) min min '()))

;; 解方程 x^2+y^2=n (x<=y)
(define (solve-sqrx-add-sqry-equal-n n)
  (solve-sqrx-add-sqry-equal-n-with-min n 1))

;; 作笛卡尔积的迭代过程
(define (descart-merge-iter a l res)
  (if (null? l)
      res
      (descart-merge-iter a (cdr l) (cons (cons a (car l)) res))))

;; 作笛卡积
;; 例如，将数1 与列表 (2 3 4) 作积得到 ((1 2) (1 3) (1 4))，顺序不保证
(define (descart-merge a l)
  (descart-merge-iter a l '()))

;; 合并列表，例如，列表(1 2 3)与(4 5)合并得到(1 2 3 4 5)，顺序不保证
(define (merge-list l1 l2)
  (if (null? l1)
      l2
      (merge-list (cdr l1) (cons (car l1) l2))))

;; 解方程 x^2+y^2+z^2=n (x<=y<=z) 的迭代过程
;; 思路是这样的，对于取定的x=a，解方程y^2+z^2=n-a^2 (a<=y<=z)得出它的解的列表，
;; 将x=a加入其中就成为原方程的解的列表
(define (solve-sqrx-add-sqry-add-sqrz-equal-n-iter n root-n x res)
  (if (>= x root-n)
      res
      (solve-sqrx-add-sqry-add-sqrz-equal-n-iter
         n
         root-n
         (+ x 1)
         (merge-list (descart-merge x (solve-sqrx-add-sqry-equal-n-with-min (- n (* x x)) x)) res))))
            
;; 解方程 x^2+y^2+z^2=n (x<=y<=z) 并限定 x 最小为 min
(define (solve-sqrx-add-sqry-add-sqrz-equal-n-with-min n root-n min res)
  (solve-sqrx-add-sqry-add-sqrz-equal-n-iter n root-n min res))

;; 解方程 x^2+y^2+z^2=n (x<=y<=z)
(define (solve-sqrx-add-sqry-add-sqrz-equal-n n)
  (solve-sqrx-add-sqry-add-sqrz-equal-n-with-min n (sqrt n) 1 '()))

;;(solve-sqrx-add-sqry-equal-n 2018)
(solve-sqrx-add-sqry-add-sqrz-equal-n 2005)
(solve-sqrx-add-sqry-add-sqrz-equal-n 2018)

