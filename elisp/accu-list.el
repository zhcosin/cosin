
(defun accu-list (list init-value op acc)
  "accumulation for list."
  (if
    list
    (accu-list
      (cdr list)
      (funcall
        acc
        init-value
        (funcall
          op
          (car list)))
      op
      acc)
    init-value))

