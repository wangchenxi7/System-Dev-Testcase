
echo "java -Xmx768M -Xms768M  -XX:+UseG1GC -XX:+PrintInterpreter -XX:SemeruMemTrace  -XX:-UseCompressedOops -XX:+PrintGCDetails -Xint ReadBarrierTest"
java -Xmx768M -Xms768M  -XX:+UseG1GC -XX:SemeruMemTrace  -XX:-UseCompressedOops -XX:+PrintGCDetails -Xint ReadBarrierTest
