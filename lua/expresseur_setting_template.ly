\version "2.24.4"

%%%% Lilypond commands
%%%% Template from expresseur_setting_temlate.ly

%%%%%%%%translate_xml_to_ly:"%s" "%s" --npl --no-beaming --output %s %s
%%%%%%%%translate_ly_to_png:"%s" -dlog-file=%s -dinclude-settings=%s -dresolution=72 -dseparate-page-formats=pdf,png  %s

%%%%%%%%override Expresseur:\override NoteHead.color = grey \override Beam.color = grey \override Flag.color = grey \override Stem.color = grey \stemUp

%%%% Set size image, according to Exresseur window sizing
#(set! paper-alist (cons '("expresseur_format" . (cons (* %d pt) (* %d pt))) paper-alist))

%%%% Set layout
\paper{
  indent=25\mm
  #(set-paper-size "expresseur_format")
  left-margin = 15\mm
  right-margin = 5\mm
  top-margin = 5\mm
  bottom-margin = 5\mm
  oddFooterMarkup=##f
  oddHeaderMarkup=##f
}

%%%% Set size notes, selecting the line according to the Expresseur zoom factor : -3 -2 -1 0 +1 +2 +3 
#(set-global-staff-size 20)
#(set-global-staff-size 25)
#(set-global-staff-size 30)
#(set-global-staff-size 35)
#(set-global-staff-size 40)
#(set-global-staff-size 45)
#(set-global-staff-size 50)



