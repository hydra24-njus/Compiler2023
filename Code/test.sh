for ir in ../Test/*.ir;
do 
#echo -e "\n\n============ ${ir#../Test/} ============\n"
./parser $ir ${ir}1;
done
