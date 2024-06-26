import QtQuick 2.0
import MuseScore 3.0
import FileIO 3.0

MuseScore {
      version:  "1.0"
      description: "plugin scan position"
      menuPath: "Plugins.Notes.scan_position"
      
      // output file to save the positions of the notes
      FileIO {
            id: outfile
            source: "__expresseur_pos.txt__" // will be replaced with the full path on runtime
            onError: console.log(msg)
      }

      // to inspect an object
      function logInspect(o, oname) {
            console.log("object " + oname + " <" + o.objectName + ">" );
            for(var p in o){
                var t=typeof o[p];
                console.log("  " + p + " / " + t )
           }
      }

      // to read positions of the chords
      function scanPosition() {

            curScore.startCmd();
            // scan the ticks of the measures
            var cursor = curScore.newCursor();
            cursor.rewind(0);
            cursor.voice = 0; 
            cursor.staffIdx = curScore.nstaves - 1;
            var tmeasure = [ ] ; 
            while (cursor.segment) {
                  tmeasure.push(cursor.tick) ;
                  cursor.nextMeasure();
            }
            // console.log("measure " + tmeasure );

            // set auto-beam to all chords
            cursor.rewind(0);
            cursor.voice = 0; 
            cursor.staffIdx = curScore.nstaves - 1;
            while (cursor.segment) {
                  // logInspect(cursor.element, "element")
                  if (cursor.element && cursor.element.type == Element.CHORD) {
            	      cursor.element.beamMode = 0 ;
                      for(var p in cursor.element.lyrics){
                         cursor.element.lyrics[p].offsetY = 3.5
                      }
                  }
                  cursor.next();
            }
            
            // For MAC Version ( for PC version : lines deleted )
             	var style = curScore.style; 
             	style.setValue("pageEvenBottomMargin", __margin__ ); // will be replaced with the margin in inch 
            	style.setValue("pageEvenLeftMargin",  __margin__ ) ;
            	style.setValue("pageEvenTopMargin" ,  __margin__ ) ;
            	style.setValue("pageOddBottomMargin" , __margin__ );
            	style.setValue("pageOddLeftMargin" ,  __margin__ );
            	style.setValue("pageOddTopMargin" ,  __margin__ );
            	style.setValue("pageWidth" , __width__ ); //  for MAC versionwill be replaced with the width in inch
            	style.setValue("pageHeight" ,  __height__ ); //  will be replaced with the height in inch
            	style.setValue("pagePrintableWidth" ,  __pwidth__ ) ;//  will be replaced with the printable width in inch
           
            curScore.endCmd();
            // startCmd endCmd to replace doLayout
            //curScore.doLayout();
            var reswrite = writeScore(curScore,"__expresseur_out.png__","png"); // will be replaced with the full path on runtime
            cursor.rewind(0);
            cursor.voice = 0; 
            cursor.staffIdx = curScore.nstaves - 1;
            var nrmeasure = 0 ;
            var nbmeasure = tmeasure.length ;
            var offsett = 0 ;
            var toWrite = "" ;
            // logInspect(cursor.measure.parent, "cursor.measure.parent")
            while (cursor.segment) {
                  var pagenr = cursor.measure.parent.parent.pagenumber;
                  if (cursor.element && cursor.element.type == Element.CHORD) {
                        var mchord = cursor.element ;
                        var mnote ;
                        for(var pnote in mchord.notes){
                              mnote = mchord.notes[pnote];
                       }
                        var t = cursor.tick ;
                        // search the measure for this tick
                        while ((nrmeasure < nbmeasure ) && (t >= tmeasure[nrmeasure] )) {
                              nrmeasure ++ ;
                              offsett = tmeasure[nrmeasure - 1];
                        }
                       //  logInspect(mnote.pagePos, "mnote.pagePos")
                        // add the information in the string to write 
                        toWrite += 
                              nrmeasure + " " +
                              (t - offsett) +  " " +
                              (pagenr + 1) +  " "   +
                              mnote.pagePos.x.toPrecision(6) +  " " +
                              mnote.pagePos.y.toPrecision(6) +  " " +
                              mnote.bbox.width.toPrecision(6) +  " " +
                              mnote.bbox.height.toPrecision(6) + "\n" ;
                  }
                  cursor.next();
            }
	    toWrite += "eof\n" ;
            // write the positions in the output file
            outfile.write(toWrite);
      }

 
      onRun: {
            //console.log("hello scan position");

            if (typeof curScore === 'undefined')
                  Qt.quit();

            scanPosition()

            Qt.quit();
         }
}
