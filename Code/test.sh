for cmm in /share/Tests1/Tests/*.cmm;
do 
echo -e "\n\n============ ${cmm#../Test/} ============\n"
var=${cmm#/share/Tests1/Tests};
./parser $cmm /share/lab3-test/${var%.cmm}.ir;
done
