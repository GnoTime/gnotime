;
; FILE:
; gtt.scm
;
; FUNCTION:
; Miscellaneous definitions for generating reports
;
; HISTORY:
; Copyright (c) 2002,2003 Linas Vepstas <linas@linas.org>
;
; This file is covered by the GPL.  Please refer to the
; GPL license for details.

; -- debugging support XXX using these crashes guile, don't know why.
; (use-modules (ice-9 debug))
; (use-modules (ice-9 stack-catch))
;  (debug-enable 'backtrace)
;  (debug-enable 'debug)
;  (read-enable 'positions)

; Various bits of syntactic sugar for showing the current (linked)
; project title and other stuff of that sort.
;
(debug-set! stack 0)
(define (gtt-show-project-title) 
        (gtt-show (gtt-project-title (gtt-linked-project))))

(define (gtt-show-project-desc) 
        (gtt-show (gtt-project-desc (gtt-linked-project))))

(define (gtt-show-project-notes) 
        (gtt-show (gtt-project-notes (gtt-linked-project))))

(define (gtt-show-basic-journal)
        (gtt-show-journal (gtt-linked-project)))


;; ---------------------------------------------------------     

;; If a report query was run, use results from that; 
;; show selected project & subprojects
;
(define (gtt-linked-or-query-results)
        (if (gtt-did-query) 
            (gtt-query-results)
            (gtt-project-subprojects (gtt-linked-project))
        )
)

;; ---------------------------------------------------------     
; Define primitives as per generic scheme
; surely these are defined in somewhere else (slib ??)
(define (xtagged-list? exp tag)
  (if (pair? exp)
      (eq? (car exp) tag)
      #f))
(define (xquoted? exp)
  (xtagged-list? exp 'quote))

; Hmm, basic guile is missing a 'string-tail' function, so add it here.
; given a string and an integer offset, return copy of string
; from the offset to the very end.
(define (string-tail str off)
   (substring str off (string-length str) ))

;; ---------------------------------------------------------     
;; prototype infrastructure for new, "type-safe" types...

(define (gtt-is-task-list-type? x) (equal? (cdr x) "gtt-task-list") )
(define (gtt-is-interval-list-type? x) (equal? (cdr x) "gtt-interval-list") )

;; ---------------------------------------------------------     
; The gtt-apply-func-list-to-obj routine is a simple utility 
; to apply a list of functions to a single gtt object,
; where a gtt object is a task, interval, or project.
; The 'function list' should be either:
;  -- a function that takes a single gtt object as an argument
;  -- a double-quoted string
; It returns a list of the result of applying each function
; to the object, omitting null results from the list
; 
; XXX FIXME the vars first_func, next_func, parent_obj,
; next_obj, result, appres, is-bill, is-hold, is-paid  
; are scoped globally to the functions being applied.   This 
; introducees potential symbol conflist.  This needds to be fixed!
;
(define (gtt-apply-func-list-to-obj func_list obj)
   (let ( (first_func (car func_list))
          (next_func  (cdr func_list)) 
        )
   (let (
          ; result is the result of the evaluation. 
          ; We compute it here to make sure we apply only once;
          ; we can use the result for tests.
          (result (if (xquoted? first_func) 
                        (cdr first_func)  ;; just something quoted
                        (first_func obj)))
        )
   (if (null? next_func)
      (if (null? result)
        '()
        (list result))
      ; if result was null, do not put it into list! 
      (if (null? result)
        (gtt-apply-func-list-to-obj next_func obj)
        (list result (gtt-apply-func-list-to-obj next_func obj)))
   )
)))
           
; The gtt-apply-func-list-to-obj-list routine is a simple 
; utility to apply a list of functions to a list of gtt objects.
; The 'function list' should be either:
;  -- a function that takes a single object as an argument
;  -- a double-quoted string
; It returns a list of the result of applying each function
; to the object, omitting null results from the list
; 
(define (gtt-apply-func-list-to-obj-list func_list obj_list) 
   (if (null? obj_list) '()
   (let ( (parent_obj (car obj_list))
          (next_obj   (cdr obj_list))
        )
   (let (
          ; appres is the result of the evaluation. 
          ; We compute it here to make sure we apply only once;
          ; we can use the result for tests.
          (appres (if (list? parent_obj)
                      (gtt-apply-func-list-to-obj-list func_list parent_obj)
                      (gtt-apply-func-list-to-obj func_list parent_obj)) 
                  )
        )
   (if (null? next_obj)
       (if (null? appres)
         '()
         (list appres))
       (if (null? appres)
           (gtt-apply-func-list-to-obj-list func_list next_obj)
           (list appres
                (gtt-apply-func-list-to-obj-list func_list next_obj))
       )
    )
))))

;; ---------------------------------------------------------     
; The gtt-show-projects is syntatic sugar for displaying a 
; project info with embedded html markup
;
(define (gtt-show-projects prj_list func_list) 
        (gtt-show (gtt-apply-func-list-to-obj-list func_list prj_list))
)
  
; The gtt-show-tasks proceedure is syntatic sugar for displaying a 
; task info with embedded html markup
;
(define (gtt-show-tasks task_list func_list) 
        (gtt-show (gtt-apply-func-list-to-obj-list func_list task_list))
)
  
; Syntactic sugar for organizing intervals.
(define (gtt-ivls task func_list)
        (gtt-apply-func-list-to-obj-list func_list (gtt-intervals task))
)

; Utility to compute elapsed time
(define (gtt-interval-elapsed interval)
        (- (gtt-interval-stop interval) (gtt-interval-start interval))
)
  
;; ---------------------------------------------------------     
; The gtt-task-billable-value-str routine will display the value of
; a task, but only if its been marked as 'billable'.

(define (gtt-task-billable-value-str task)
            (gtt-task-value-str task)
)

;; ---------------------------------------------------------     
;; Define some filters to prune down task lists.
;;
;; The gtt-billable-tasks takes a list of tasks, and returns
;; a list of only those that are billable.

(define (gtt-filter-bill-tasks tasks)
        (define (is-bill task)
                (if (equal? (gtt-task-billstatus task) (gettext '"Bill"))
                  task  '())
        )
        (gtt-apply-func-list-to-obj-list (list is-bill) tasks)
)

(define (gtt-filter-paid-tasks tasks)
        (define (is-paid task)
                (if (equal? (gtt-task-billstatus task) (gettext '"Paid"))
                  task  '())
        )
        (gtt-apply-func-list-to-obj-list (list is-paid) tasks)
)

(define (gtt-filter-hold-tasks tasks)
        (define (is-hold task)
                (if (equal? (gtt-task-billstatus task) (gettext '"Hold"))
                  task  '())
        )
        (gtt-apply-func-list-to-obj-list (list is-hold) tasks)
)

;; ---------------------------------------------------------     
; The below identifies a 'daily-obj' type, with getters for its two members.
; The first member is the date,
; The second member is the amount of time spent on the project on that date.
; At this point, both members are strings; this may change someday.

(define (gtt-is-daily-type? daily-obj)  (equal? (cdr daily-obj) "gtt-daily") )

;; XXX should really be using srfi-19 to handle the date printing
(define (gtt-daily-day-str  daily-obj)  
        (if (gtt-is-daily-type? daily-obj)
            (caar daily-obj) ))
            
(define (gtt-daily-time-str daily-obj)  
        (if (gtt-is-daily-type? daily-obj)
            (cadar daily-obj) ))

(define (gtt-show-daily dly_list func_list)
        (gtt-show  (gtt-apply-func-list-to-obj-list func_list dly_list)))

;; ---------------------------------------------------------     
;; Return the task list part of the daily report.
;; Currently, it assumes that the object is in a fixed position
;; in the list.  A better implementation would perform a search
;; in the list for the right type.
(define (gtt-daily-task-list daily-obj)
        (if (gtt-is-daily-type? daily-obj)
             (caddar daily-obj) ))

(define (gtt-daily-interval-list daily-obj)
        (if (gtt-is-daily-type? daily-obj)
             (cadr (cddar daily-obj) )))

;; ---------------------------------------------------------     
; Syntactic sugar that allows various task attributes to 
; be extracted next to each other ... see daily report for usage
(define (gtt-show-daily-tasks  dailyobj tasklist)  
        (gtt-apply-func-list-to-obj-list
             tasklist
             (car (gtt-daily-task-list dailyobj) )
         ))

;; ---------------------------------------------------------     
; plain-text to HTML beautification.  These filters can take
; plain-text input strings, and mark them up so that the
; formatting is preserved in html.

; Convert newlines to <br>\n and return the result.
; this routine is implemented tail-recursively
(define (gtt-html-escape-newline str)
   (define (tail-escape-newline accum str)
      (let ((noff (string-index str #\newline))
           )
      (if (equal? #f noff) 
           (string-append accum str)
           (tail-escape-newline  
                  (string-append accum (substring str 0 noff) "<br>\n")
                  (string-tail str (+ noff 1))
           )
      )
   ))

  (tail-escape-newline "" str)
)

; For right now, all that this does is the newline thing.
; Hopefully it will do the URL markup too soon.
(define (gtt-html-markup str) 
    (gtt-html-escape-newline str)
)

;; ---------------------------------------------------------     
;; --------------------- end of file -----------------------     
