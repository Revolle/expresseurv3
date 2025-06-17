\version "2.24.4"

%%%% Lilypond commands
%%%%%%%%translate_xml_to_ly:%s %s --npl --no-beaming --output=%s %s
%%%%%%%%translate_ly_to_png:%s -dlog-file=%s -dinclude-settings=%s -dresolution=72 -dseparate-page-formats=pdf,png  --output %s %s

%%%% Set size image, according to Exresseur window sizing
#(set! paper-alist (cons '("expresseur_format" . (cons (* %d pt) (* %d pt))) paper-alist))

%%%% Set layout
\paper{
  indent=2\mm
  #(set-paper-size "expresseur_format")
  oddFooterMarkup=##f
  oddHeaderMarkup=##f
}

%%%% Set size notes, selecting the line according to the Expresseur zoom factor : -3 -2 -1 0 +1 +2 +3 
#(set-global-staff-size 30)
#(set-global-staff-size 35)
#(set-global-staff-size 40)
#(set-global-staff-size 45)
#(set-global-staff-size 50)
#(set-global-staff-size 55)
#(set-global-staff-size 60)

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%% capture point-and-click position of Expresseur notes 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

#(define exp-output-port (open-output-file "expresseur_out.lyp" ))

#(define (format-note engraver event)
   (let* ((origin (ly:input-file-line-char-column
                   (ly:event-property event 'origin)))
          (pitch (ly:event-property event 'pitch)))
     ( if ly:pitch? (display (ly:format "p:~a:~a:\n" (cadr origin) (caddr origin)) exp-output-port ))
     ))
	 
#(define event-listener-engraver
  (make-engraver
    (listeners
     (note-event . format-note) ;; works for notes
     )))

\layout {
  \context {
    \DrumStaff
    \consists #event-listener-engraver
  }
}

