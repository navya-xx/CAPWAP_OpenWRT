<?php
	$channel = $_GET['channel'];
	if ($channel=='') $channel=-1;

	$txpower = $_GET['txpower'];
	if ($txpower=='') $txpower=-1;
	
	$version = $_GET['version'];
	if ($version=='') $version=-1;
	
	$wtpid = $_GET['WTP'];
	$cmd_pwd = "cd /home/navneet/workspace/capwap-demo/html/";
	exec ($cmd_pwd, $text_pwd, $retVal);
	if($retVal != 0) {
		print ("System command exec(pwd) failed! ERROR!");
		exit();
	}
	$cmd_tar = "./create-tar-chan-txpower.sh $channel $txpower $version";
//echo $cmd_tar;
	exec ($cmd_tar, $test_tar, $retVal);

	if($retVal != 0) {
		print ("System command exec(tar) failed! ERROR!");
		exit();
	}
	
	//var_dump($test_tar);

	$cmd_wum = "cd /home/navneet/workspace/capwap-demo/wum/ \n ./wum -c update -w $wtpid -f /tmp/capwap-tmp.tar.gz";
	//echo $cmd_wum;
	exec($cmd_wum, $test_cmd, $retVal);
	
	if($retVal != 0) {
		print("System command exec(set) failed! ERROR!");
		exit();
	}
	var_dump($test_cmd);


	$val = $test_cmd[3];
	print $val;
	$wtp_arr = explode("|", $val);
	$wtp_id = trim ( $wtp_arr[1] , " \t\n\r\0\x0B");
	if($wtp_id != $wtpid) {
		print ("WTP id doesn't match!");
		exit();
	}
	$compare = trim ( $wtp_arr[3] , " \t\n\r\0\x0B");
	print $compare;
	if($compare == 'SUCCESS'){
		print (1);
	} else {
		print (0);
	}
	


?>
