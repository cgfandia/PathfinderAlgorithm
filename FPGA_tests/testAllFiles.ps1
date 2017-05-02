$files = Get-ChildItem placed
$pathfinderEXE = '../Release/Pathfinder.exe'
$resFolder = './results'
$edgeWeight = 1000;
$channelCapacity = 7;
$Fvp = 0.00001;
$Fvh = 0.1;
$iterCount = 500;
$i = 0;
$channelCapacityArray = 10, 11, 12, 7, 12, 8, 8, 7, 10, 10, 12, 12, 11, 16, 7, 8, 9, 11, 13, 7
if(!(Test-Path $resFolder)){
	New-Item $resFolder -ItemType "directory"
}
foreach($f in $files){
	<#if($i -ge 1){
		exit
	}#>
	$channelCapacity = $channelCapacityArray[$i]
	$circuitName = $f.BaseName -replace "\.place$"
	$netFile = ($f.DirectoryName -replace "placed", "net") + "\" `
	+ $circuitName + ".net"
	$resFileName = $resFolder + "/${circuitName}_${edgeWeight}_${channelCapacity}_${Fvp}_${Fvh}_${iterCount}.txt"	
	Write-Host $resFileName
	"circuit = $circuitName,edgeWeight = $edgeWeight,channelCapacity = $channelCapacity,Fvp = $Fvp,Fvh = $Fvh,iterCount = $iterCount" > $resFileName
	(&$pathfinderEXE $f.FullName $netFile $edgeWeight $channelCapacity $Fvp $Fvh $iterCount) >> $resFileName
	$i++
}
