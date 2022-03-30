
PA_DIR=$1

DIR="/usr/lib64 /usr/lib /lib /lib64 /usr/lib/x86_64-linux-gnu"
AS=`find $DIR -maxdepth 2 -name 'libasound.so*' | head -n1`
if LANG=C gcc -lportaudio 2>&1 | grep -q "cannot find -lportaudio"; then
  echo $PA_DIR"/libportaudio.a $AS -lrt -ldl"
else
  echo "-lportaudio"
fi
