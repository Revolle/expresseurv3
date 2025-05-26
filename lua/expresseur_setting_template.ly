\version "2.24.4"

%%%% write the lilypond-score positions of notes on a text efile

(define file_pos_exp null)

#(define (format-note engraver event)
   (
		let* 
		(
			(inst-name (ly:context-property context 'instrumentName))
			(pitch (ly:event-property event 'pitch))
			(origin (ly:input-file-line-char-column (ly:event-property event 'origin)))
		)
		(
			if (and (string=? inst-name "_X_" ) (ly:pitch? pitch))
			(
				display				
					( ly:format "p:~a:~a:" (caddr origin) (cadr origin) )
					file_pos_exp
			)
		)
	)
)

#(define event-listener-engraver
  (
	make-engraver
	(
		listeners
		(
			note-event . format-note
		)
     )
   )
)

\layout 
{
	\context 
	{
        \Score
        % Call the Scheme function at the start
        \override Score.StartTextSpan.before-line-breaking =
          #(
				lambda (grob)
				(
					set! file_pos_exp (open-file "expresseur_out_ly.notes" "w")
				)
			)
        % Call the Scheme function at the end
           \override Score.EndTextSpan.after-line-breaking =
          #(
				lambda (grob)
				(
					close file_pos_exp
				)
			)
	}  
	\context 
	{
		\Voice
		\consists #event-listener-engraver
	}
}
