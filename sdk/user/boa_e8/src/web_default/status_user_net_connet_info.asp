<!-- add by liuxiao 2008-01-16 -->
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<html>
<head>
<title>中国电信</title>
<meta http-equiv=pragma content=no-cache>
<meta http-equiv=cache-control content="no-cache, must-revalidate">
<meta http-equiv=content-type content="text/html; charset=gbk">
<meta http-equiv="refresh" content="5">
<meta http-equiv=content-script-type content=text/javascript>
<style type=text/css>
@import url(/style/default.css);
</style>
<script language="javascript" src="common.js"></script>
<SCRIPT language="javascript" type="text/javascript">
var links = new Array();
<% listWanConfig(); %>
/********************************************************************
**          on document load
********************************************************************/
function on_init()
{
	if(lstrc.rows){while(lstrc.rows.length > 1) lstrc.deleteRow(1);}
	for(var i = 0; i < links.length; i++)
	{
		var row = lstrc.insertRow(i + 1);

		row.nowrap = true;
		row.vAlign = "top";
		row.align = "center";

		var cell = row.insertCell(0);
		cell.innerHTML = links[i].servName;
		cell = row.insertCell(1);
		cell.innerHTML = links[i].strStatus;
		cell = row.insertCell(2);
		cell.innerHTML = links[i].ipAddr;
	}
}

</SCRIPT>

</head>
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init();">
	<blockquote>
		<div align="left" style="padding-left:20px;"><br>
			<div align="left"><b>WAN信息</b></div>
			<table class="flat" id="lstrc" border="1" cellpadding="1" cellspacing="1" width="90%">
				<tr class="hdb" align="center" nowrap>
					<td>服务接口</td><td>状态</td><td>IP地址</td>
				</tr>
			</table>
			<br><br>
			<b>网络信息</b>
			<br>
			<table class="flat" border="1" cellpadding="1" cellspacing="1">
				<tr>
					<td width="20%" class="hdb">缺省网关</td>
					<td><% getDefaultGW(); %></td>
				</tr>
				<tr>
					<td class="hdb">子网掩码</td>
					<td><% getDefaultGWMask(); %></td>
				</tr>
				<tr>
					<td class="hdb">首选DNS服务器</td>
					<td><% getInfo("wan-dns-1"); %></td>
				</tr>
				<tr>
					<td class="hdb">备用DNS服务器</td>
					<td><% getInfo("wan-dns-1"); %></td>
				</tr>
			</table>
		</div>
	</blockquote>
</body>
<%addHttpNoCache();%>
</html>
