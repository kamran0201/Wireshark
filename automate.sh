#!/bin/bash

reset;
echo "Cleaning files if already present";
rm -rf send.txt;
rm -rf recv.txt;
touch send.txt;

echo "Creating send.txt of size approximately 5MB";
str="random gibberish lorem ipsum 252";
len=32;
# len(str) = 32. space used up = 32 * space per char = 32 bytes
# Number of repeats = 5 MB / 32 B = 5 * 1024 kB / 32 B= 5 * 1024 * 1024 B / 32 B = 163840
for((i=0;i<163840; i++))
do
echo "$str" >> send.txt;
done;

echo "Creating arrays of TCP, delays, losses";
tcp_names=("cubic" "reno");
delay=("10ms" "50ms" "100ms");
loss=("0.1%" "0.5%" "1%");
recv_port=$1;
gcc client.c -o client;
gcc server.c -o server;

echo "Setting MTU size on loopback interface (lo on my laptop) to 1500";
# Delete earlier rule (if it is not present, it will simply say device not found. Ignore that)
sudo tc qdisc del dev lo root;
# Set mtu size = 1500
sudo ifconfig lo mtu 1500;

echo "Looping over 3 x 2 x 2 different experiments (20 runs for each)";
echo "";
# Loop over the experiments
for i in "${tcp_names[@]}"
do
for j in "${delay[@]}"
do
for k in "${loss[@]}"
do
# Add rule
sudo tc qdisc add dev lo root netem delay "$j" loss "$k";
# Check if rule has been added
sudo tc qdisc show dev lo;
# Create file for storing results of this experiment
file_name="results_delay_""$j""_loss""_""$k""_tcp""_"$i".txt";
equal=0;
nonequal=0;
rm -rf "$file_name";
touch "$file_name";
echo "Running experiment with parameters : TCP variant = $i, Delay = $j, Loss = $k";
# Run experiment with given parameters
for((count=0; count < 20; count++)) 
do
{
./server $recv_port $j $k $i $file_name;
}&
pid_1=$!;
sleep 0.5s;
{
./client $recv_port $j $k $i $file_name;
}&
pid_2=$!;
# Wait for both processes to get over
wait $pid_2;
wait $pid_1;
# Check if send.txt = recv.txt. Update counts accordingly.
diff send.txt recv.txt | read -r equality_check;
if [[ -z $equality_check ]];
then
equal=$(($equal+1));
else
nonequal=$(($nonequal+1));
fi
# Show progress
echo -ne "\rProgress [";
for((counter = 0; counter <= $count; counter++))
do
echo -ne "#";
done;
for((counter = $count + 1; counter < 20; counter++))
do
echo -ne " ";
done
progress=$(($count*5));
#echo -ne "($progress%)";
echo -ne " ($(($count+1))/20)";
echo -ne "]";
# Increment sender port by 1 because current sender port will be in TIME_WAIT period for some time
recv_port=$(($recv_port+1));
done
# Show progress = 20/20
echo -ne "\rProgress [#################### (20/20)]";
echo "";
# Show count of equal send.txt and recv.txt and show count of unequal files
echo "#times send.txt = recv.txt is $equal";
echo "#times send.txt != recv.txt is $nonequal";
echo "";
# Delete rule. Modifying it did not work for some reason. Deleting and adding worked.
sudo tc qdisc del dev lo root;
done
echo "";
done
echo "";
done

echo "Calling python script to make required graphs";
# Create required graphs using this .py file
python3 make_graphs.py;
