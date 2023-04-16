for cmm in ../Test/*.cmm;
do 
echo -e "\n\n============ ${cmm#../Test/} ============\n"
var=${cmm#../Test/};
./parser $cmm > /share/lab3-test/${var%.cmm}.output;
done
