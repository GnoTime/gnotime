;
; FILE:
; gtt.scm
;
; FUNCTION:
; Miscellaneous definitions for generating reports
;
; HISTORY:
; Copyright (c) 2002 Linas Vepstas <linas@linas.org>
;
; This file is covered by the GPL.  Please refer to the
; GPL license for details.


; Various bits of syntactic sugar for showing the current (linked)
; project title and other stuff of that sort.
;
(define (gtt-show-project-title) 
        (gtt-show (gtt-project-title (gtt-linked-project))))

(define (gtt-show-project-desc) 
        (gtt-show (gtt-project-desc (gtt-linked-project))))

(define (gtt-show-project-notes) 
        (gtt-show (gtt-project-notes (gtt-linked-project))))

(define (gtt-show-basic-journal)
        (gtt-show-journal (gtt-linked-project)))


;; ---------------------------------------------------------     
; Define primitives as per generic scheme
; surely these are defined in somewhere else (slib ??)
(define (xtagged-list? exp tag)
  (if (pair? exp)
      (eq? (car exp) tag)
      #f))
(define (xquoted? exp)
  (xtagged-list? exp 'quote))


;; ---------------------------------------------------------     
; The gtt-apply-func-list-to-proj routine is a simple utility 
; to apply a list of functions to a single project.
; The 'function list' should be either:
;  -- a function that takes a single project as an argument
;  -- a double-quoted string
; 
(define (gtt-apply-func-list-to-proj func_list prj)
   (let ( (first_func (car func_list))
          (next_func  (cdr func_list)) 
        )
        (list
           ; if project is really a list, then handle that
           (if (list? prj)
               (gtt-apply-func-list-to-proj-list func_list prj))

           ; if its not a function, but just a quoted string,
           ; then output that string
           (if (xquoted? first_func) 
               (cdr first_func)
               (first_func prj))
               
           (if (not (null? next_func))
               (gtt-apply-func-list-to-proj next_func prj))
        )
))
           
; The gtt-apply-func-list-to-proj routine is a simple utility 
; to apply a list of functions to a list of projects.
; The 'function list' should be either:
;  -- a function that takes a single project as an argument
;  -- a double-quoted string
; 
(define (gtt-apply-func-list-to-proj-list func_list prj_list) 
   (let ( (parent_proj (car prj_list))
          (next_proj   (cdr prj_list))
        )
   
        (list
           ; if parent is a list, then its a list of child projects
           (if (list? parent_proj)
               (gtt-apply-func-list-to-proj-list func_list parent_proj))

           ; if parent is a singleton, then its a projet 
           ; apply the column functions to it
           (if (not (pair? parent_proj))
               (gtt-apply-func-list-to-proj func_list parent_proj)) 
               
           ; and if there are more projects, do them
           (if (not (null? next_proj))
                (gtt-apply-func-list-to-proj-list func_list next_proj))
        )
))

; The gtt-show-projects is syntatic sugar for displaying a 
; project info with embedded html markup
;
(define (gtt-show-projects prj_list func_list) 
        (gtt-show (gtt-apply-func-list-to-proj-list func_list prj_list))
)
  
