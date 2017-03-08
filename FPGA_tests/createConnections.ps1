$maxConnections = 1000
$maxLengthOfConnection = 5
$maxID = 76000
$minID = 1
$fileName = "slashdotConn.txt"
for($i = 0 ; $i -lt $maxConnections ; $i++){
	
	for($j = 0 ; $j -lt (get-random -Maximum $maxLengthOfConnection -Minimum 2) ; $j++){
		$ID = get-random -Maximum $maxID -Minimum $minID
		$connection += "$ID " 
	}
	$connection >> $fileName
	
	#echo $connection
	clear-variable connection
}