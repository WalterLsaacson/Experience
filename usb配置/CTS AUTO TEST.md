* 需要修改的文件

```shell
/etc/udev/rules.d
1. 51-MTKinc.rules
UBSYSTEM=="usb", SYSFS{idVendor}=="0e8d", MODE="0666"
SUBSYSTEM=="usb", ATTR{idVendor}="0e8d", ATTR{idProduct}="201d", SYMLINK+="android_adb"
KERNEL=="ttyACM*", MODE="0666"
2.51-android.rules
#SUBSYSTEM=="usb",ATTR{idvendor}=="0e8d",MODE=="201C",GROUP="plugdev"
SUBSYSTEM=="usb",ATTR{idVendor}=="0e8d",ATTR{idProduct}=="201c",MODE="0666",OWNER="lava"
SUBSYSTEM=="usb",ATTR{idVendor}=="0e8d",ATTR{idProduct}=="201d",MODE="0666",OWNER="lava"
SUBSYSTEM=="usb",ATTR{idVendor}=="18d1",ATTR{idProduct}=="4ee7",MODE="0666",OWNER="lava"
SUBSYSTEM=="usb",ATTR{idVendor}=="1782",ATTR{idProduct}=="4002",MODE="0666",OWNER="spr"
#SUBSYSTEM=="usb", SYSFS{idVendor}=="0e8d", MODE="0666"
#SUBSYSTEM=="usb", ATTR{idVendor}="0e8d", ATTR{idProduct}="201d", SYMLINK+="android_adb"
3.重启udev进程让修改生效sudo /etc/init.d/udev restart
```

* 自动化刷机脚本

```shell
#!/bin/bash
#different flash tools depend on project names

#make usb port useful
#source /etc/udev/rules.d/50-android.rules
(sleep 30;adb reboot;)&
#flash
export FLASH_DA=/home/guanyin/workspace/flash/MTK_AllInOne_DA.bin
export FLASH_SCATTER=/home/guanyin/server/workspace/op67/out/target/product/lava6761_32_op67/MT6761_Android_scatter.txt
cd SP_Flash_Tool_exe_Linux_v5.1816.00.100/
echo 'guanyin1010' | sudo -S ./flash_tool -d $FLASH_DA -s $FLASH_SCATTER -c download -b -r
```

* 自动跑CTS的脚本

```shell
#will gen adbs.txt,frefile.

#more debug info
#set -e
#set -x


#prepare environment
source /etc/profile


function prepare {
    #save adb devices serial numbers to adbs.txt
    rm -f adbs.txt
    adb devices | grep -e "[0-9][A-Z]" | while read line; do serial=$(echo $line | cut -d " " -f 1);echo $serial >> adbs.txt; done;
    
    for serial in $(cat adbs.txt)
    do
        preconditions $serial
    done
    
    cd /home/workspace/8.1-r7/android-cts-8.1_r7-linux_x86-arm/android-cts/tools
}

function preconditions {
if [ true = $PRECONDITIONS ]
then
	#recovery device
	adb -s $1 shell dumpsys recovery reset-factory
	sleep 480

	#stay_awake
	adb -s $1 shell settings put global stay_on_while_plugged_in 7

	#input keyevent 27
	adb  -s $1 shell input keyevent 27

	#copy media file
    #TODO redirct log to 2>&1 > /dev/null
	cd /home/workspace/android-cts-media-1.4
	./copy_media.sh -s $1 > /dev/null 
	./copy_images.sh -s $1 > /dev/null 
fi
}

function  run_item {
cd ../results
#use the result of subplan to test each function
file_list=$(ls -rh $(find -name test_result.xml))
newest_result=$(echo $file_list | cut -d " " -f 1)
echo $newest_result

if [ ! -d "../jenkins_subplan_items" ]
then
    mkdir ../jenkins_subplan_items
fi
                
cp -f $newest_result ../jenkins_subplan_items/
cd ../jenkins_subplan_items

rm -f ../tools/cts_commands.txt

#make dir
rm -rf failed
mkdir failed
rm -rf passed
mkdir passed

#module,testcase,fun
module=""
testcase=""
fun=""

while  read -r line
do
    echo $line
    #filter the test case and gen cts commands
	if [ -n "$(echo $line | grep "<Module ")" ]
	then
            module=$(echo ${line##*"name=\""} | cut -d "\"" -f 1)
            #echo $module
	fi
	if [ -n "$(echo $line | grep "\<TestCase ")" ]
	then
	    testcase=$(echo ${line##*"name=\""} | cut -d "\"" -f 1)
            #echo $testcase
	fi
	if [ -n "$(echo $line | grep "<Test ")" ]
    then
    	if [ -n "$(echo $line | grep "fail")" ]
		then
            fun=$(echo ${line##*"name=\""} | cut -d "\"" -f 1)
            #echo $fun
            command="run cts -m $module -t $testcase#$fun -o -d"
            echo command:$command
            echo $command >> ../tools/cts_commands.txt
            
            #run cts command and save log
            cd ../tools
            #1.choose a specific device
            specific_device=$(cat adbs.txt | tail -n 1)
            rm -f cts_log.log
            #2.catch cts log
            ./cts-tradefed $command -s $specific_device --logcat-on-failure > cts_log.log
            logcat=$(ls -rh $(find ../logs/ -name device_logcat_test*) | head -1)
            #3.judge pass or fail
            passed=$(cat cts_log.log | grep "PASSED: " | cut -d "," -f 1 | cut -d "." -f 2)
            	#failed
            	if [ "PASSED: 0" = "$(echo $passed)" ]
            	then
            		#find failed
                	if [ ! -d "../jenkins_subplan_items/failed/$fun" ]
                	then
                    	mkdir ../jenkins_subplan_items/failed/$fun
                	fi
            		#mv mtklog/ ../jenkins_subplan_items/failed/$fun/
                    cp $logcat ../jenkins_subplan_items/failed/$fun/
            		mv cts_log.log ../jenkins_subplan_items/failed/$fun/
            	fi
            	#passed
            	if [ "PASSED: 1" = "$(echo $passed)" ]
            	then
                	if [ ! -d "../jenkins_subplan_items/failed/$fun" ]
                	then
            	    	mkdir ../jenkins_subplan_items/passed/$fun
                	fi
            		mv cts_log.log ../jenkins_subplan_items/passed/$fun/
                    cp $logcat ../jenkins_subplan_items/passed/$fun/
                	#mv mtklog/ ../jenkins_subplan_items/passed/$fun/
            	fi
            fi
            cd ../jenkins_subplan_items/
        fi
done < test_result.xml
#item_read_run
}

function item_read_run {
            while read line
            do
                #clear mobile log
            	adb -s $specific_device shell am broadcast -a com.mediatek.mtklogger.ADB_CMD -e cmd_name clear_all_logs
            	sleep 5
                #start mobile log
            	adb -s $specific_device shell am broadcast -a com.mediatek.mtklogger.ADB_CMD -e cmd_name start --ei cmd_target 1
            	sleep 5
                #stop mobile log
            	adb -s $specific_device shell am broadcast -a com.mediatek.mtklogger.ADB_CMD -e cmd_name stop --ei cmd_target 1
            	sleep 5
            	adb -s $specific_device pull sdcard/mtklog
            	#./cts-tradefed $command -s $specific_device > cts_log.log
            	#judge failed or passed
            	#2.before rerun this fun open mtklog,save it when test done
           		#3.save cts log
                done
}

cd /home/workspace/8.1-r7/android-cts-8.1_r7-linux_x86-arm/android-cts/tools
#run cts_commadns concurrent
FRE=$(cat frefile)
echo FRE:$FRE

if [ $FRE -ge 0 ]
then
    #run subplan
    if [ 6 -eq $FRE ]
    then
        echo "run subplan.failed & unexe."
        prepare
        #want to run subplan, one of passed, failed, not_executed.
        subplan_name=$(date "+%Y_%m_%d@%H:%M:%S")failed_noexe
        #TODO: Part failed items and unexecuted items
        ./cts-tradefed add subplan --session 6 --name $subplan_name --result-type failed --result-type not_executed
        ./cts-tradefed run cts --subplan $subplan_name
        FRE=$[  $FRE + 1 ]
        echo $FRE > frefile
        exit 0
    fi
    
    #run failed item
    if [ 7 -eq $FRE ]
    then
        prepare

        #./cts-tradefed command
        run_item
        echo -1 > ../tools/frefile
        exit 1
    fi
    
    #run retry
    echo This is $FRE times retry
    prepare
    ./cts-tradefed run cts  --retry $FRE --shard-count $DEVICE_COUNT
    FRE=$[  $FRE + 1 ]
    echo $FRE > frefile
else
    #run first time
    echo Try first time
    prepare 
    ./cts-tradefed run cts --shard-count $DEVICE_COUNT #-l debug
    echo 0 > frefile
fi
```

