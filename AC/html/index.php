<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
<head>
	<title>TICET : WLAN Management</title>
	<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
	<link rel="stylesheet" type="text/css" href="style.css" />
<style type="text/css">
<!--
body {
	background-color: #ffffff;
}
#wtp_list_tab {
	border: 1px solid black;
	
}
#wtp_list_tab th, td{
	padding: 5px;
	border: 1px dotted black;
}
-->
</style>
<script src="jquery-1.11.1.min.js"></script>
<script type="text/javascript">
	$(document).ready(function(){
      //refreshTable();
		get_ajax_wtps();
	});
	
	function get_ajax_wtps(){
		var url = "wtp_list.php";
		$.ajax(url).done(function(data) {
					var json_var = $.parseJSON(data);
					if(data=='0'){
						$("#WTP_LIST").append("NO WTP found!!");
					} else {
						display_wtps(json_var);
					}
				});
	}
	function display_wtps(wtp_arry){
		var wtp_dom = $("#WTP_LIST");
		var table_dom = "<table id='wtp_list_tab'><tr>";
		table_dom += "<th>WTP ID</th>";
		table_dom += "<th> WTP Name </th>";
		table_dom += "<th> Version </th>";
		table_dom += "<th> Info </th>";
		table_dom += "<th> Set Channel </th>";
		table_dom += "<th> Set Tx Power </th>";
		table_dom += "<th> Execute </th>";
		table_dom += "</tr>";		
		$.each(wtp_arry, function(index, val) {
			var val_arr = val.split("|");
			table_dom += "<tr>";
			table_dom += "<td>"+index+"</td>";
			table_dom += "<td><strong>"+val_arr[0]+"</strong></td>";
			table_dom += "<td><span class='version'>"+val_arr[1]+"</span></td>";
			table_dom += "<td>"+get_wtp_txp(index)+"</td>";
			table_dom += "<td><select id='channel'><option value=0 selected>Channel</option>";
				<?php 
						$html_list="";
						for($i=1; $i<12; $i++) { 
							$html_list.='<option value='.$i.'>'.$i.'</option>';
						}
				?>
			table_dom += "<?php echo $html_list; ?>";
			table_dom +="</select></td>";
			table_dom += "<td><input size=5 type='text' class='txpower' /></td>";
			table_dom += "<td><input type='button' value='Set' onclick='set_freq_txp(this, "+index+")' /></td>";
			table_dom += "</tr>";
		});	
			table_dom += "</table>";
		$(wtp_dom).html(table_dom);
		$("#WTP_LIST").append("<br/><input type='button' onclick='get_ajax_wtps()' value='Refresh WTP List' />");
	}
	
	function set_freq_txp(wtp_dom, index){
		var channels = $(wtp_dom).parent().siblings().children("#channel").val();
		var txpowers = $(wtp_dom).parent().siblings().children('.txpower').val();
		var ver_sion = $(wtp_dom).parent().siblings().children('span.version').html();
		var urls = "wtp_set.php";
		
		$.ajax({
				type: 'GET',
				url: urls,
				data: {channel: channels, txpower: txpowers, version: ver_sion, WTP: index, rand:Math.random()}
			}).done(function(data) {
					if(data == 0){
						alert("Set failed!!");
					}
					else {
						alert("Set Successful!!");
					}
				});
	}

	function get_wtp_freq(wtp_index){
		return "--";
	}

	function get_wtp_txp(wtp_index){
		return "--";
	}
		
</script>
</head>
<body>
<div id="page"> 
    <div id="header">
    	<div class="title">TICET</div>
        <div class="subText">WLAN Management</div>
    </div>
    <div id="pageContent">
		<div id="WTP_LIST">
		
		</div>
	</div>
</div>        
</body>
</html>
