;
; FILE:
; gtt.scm
;
; FUNCTION:
; Miscellaneous definitions for generating reports
;
;

(define (gtt-show-project-title) 
        (gtt-show (gtt-project-title (gtt-selected-project))))

(define (gtt-show-project-desc) 
        (gtt-show (gtt-project-desc (gtt-selected-project))))

(define (gtt-show-project-notes) 
        (gtt-show (gtt-project-notes (gtt-selected-project))))

(define (gtt-show-basic-journal)
        (gtt-show-journal (gtt-selected-project)))
