
PA_DIR=$1

DIR="/usr/lib64 /usr/lib"
AS=`find $DIR -maxdepth 2 -name 'libasound.so*' | head -n1`
echo $PA_DIR"/libportaudio.a $AS -lrt -ldl"
