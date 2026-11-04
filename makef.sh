touch expresseur/expresseur.cpp
echo "**********************************"
echo "rm old Expresseur.app"
echo "**********************************"
rm -r expresseur/Expresseur.app
#rm -r /Applications/Expresseur.app
echo "**********************************"
echo "make"
echo "**********************************"
make
#echo "**********************************"
#echo "cp Expresseur.app in /applications"
#echo "**********************************"
#cp -R expresseur/Expresseur.app /Applications
#xecho "**********************************"
#echo "xattr -dr Expresseur.app in /applications"
#echo "**********************************"
#xattr -dr com.apple.quarantine /Applications/Expresseur.app
