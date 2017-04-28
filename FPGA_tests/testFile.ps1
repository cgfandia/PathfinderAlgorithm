$file = Get-ChildItem -Path './placed/alu4.place'
$pathfinderEXE = '../Release/Pathfinder.exe'
$resFolder = './results'
$edgeWeight = 100;
$channelCapacity = 7;
$Fvp = 1.2;
$Fvh = 0.3;
$iterCount = 300;

$FvpArray = 0.001, 0.01, 0.1, 1, 1.5 # Fvp Test parameters
$edgeWeightArray = 1, 10, 100, 500 # Fvp Test parameters

if(!(Test-Path $resFolder)){
	New-Item $resFolder -ItemType "directory"
}

foreach($P in $FvpArray){
	foreach($F in $FvpArray){
		$Fvp = $P
		$Fvh = $F
		$circuitName = $file.BaseName -replace "\.place$"
		$netFile = ($file.DirectoryName -replace "placed", "net") + "\" `
		+ $circuitName + ".net"
		$resFileName = $resFolder + "/${circuitName}_${edgeWeight}_${channelCapacity}_${Fvp}_${Fvh}_${iterCount}.csv"	
		Write-Host $resFileName
		"circuit = $circuitName,edgeWeight = $edgeWeight,channelCapacity = $channelCapacity,Fvp = $Fvp,Fvh = $Fvh,iterCount = $iterCount" > $resFileName
		(&$pathfinderEXE $file.FullName $netFile $edgeWeight $channelCapacity $Fvp $Fvh $iterCount) >> $resFileName
	}
}
