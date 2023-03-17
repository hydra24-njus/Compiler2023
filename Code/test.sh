for cmm in ../Test/*.cmm;
do 
echo -e "\n\n============ ${cmm#../Test/} ============\n"
./parser $cmm > ${cmm%.cmm}.output;
done
