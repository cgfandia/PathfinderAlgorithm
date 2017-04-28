$files = Get-ChildItem placed
$pathfinderEXE = '../Release/Pathfinder.exe'
$resFolder = './results'
$edgeWeight = 100;
$channelCapacity = 7;
$Fvp = 0.01;
$Fvh = 0.001;
$iterCount = 200;
$i = 0;
if(!(Test-Path $resFolder)){
	New-Item $resFolder -ItemType "directory"
}
foreach($f in $files){
	<#if($i -ge 1){
		exit
	}#>
	$circuitName = $f.BaseName -replace "\.place$"
	$netFile = ($f.DirectoryName -replace "placed", "net") + "\" `
	+ $circuitName + ".net"
	$resFileName = $resFolder + "/${circuitName}_${edgeWeight}_${channelCapacity}_${Fvp}_${Fvh}_${iterCount}.txt"	
	Write-Host $resFileName
	"circuit = $circuitName,edgeWeight = $edgeWeight,channelCapacity = $channelCapacity,Fvp = $Fvp,Fvh = $Fvh,iterCount = $iterCount" > $resFileName
	(&$pathfinderEXE $f.FullName $netFile $edgeWeight $channelCapacity $Fvp $Fvh $iterCount) >> $resFileName
	#$i++
}
