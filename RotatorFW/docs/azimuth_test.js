function rawRotatorPositionToAzimuth(sensed) {
	// sensed 0 - 449
	// out 0 - 359

	return (rotatorStartPoint + sensed) % 360;
}

function rotatorSetTargetAzimuth(currentRotatorPosition, targetA, shouldBe) {
	// target 0 - 359
	// console.log('targetA', targetA);

	targetA += 360;

	// console.log('currentRotatorPosition', currentRotatorPosition);
	const sourceA = rawRotatorPositionToAzimuth(currentRotatorPosition);
	// console.log('sourceA', sourceA);
	let diff = targetA - sourceA;
	// console.log('diff 1', diff);
	diff = (diff + 180) % 360 - 180;
	// console.log('diff 2', diff);

	let finalPos = currentRotatorPosition + diff;
	// console.log(finalPos);

	if (finalPos < 0) {
		diff += 360;
	}

	if (finalPos > rotatorMotionRange) {
		diff -= 360;
	}

	const formatDir = (n => (n === 0 ? '0   ' : ((n < 0 ? '-' : '+') + String(Math.abs(n)).padStart(3, '0'))));
	console.log(
		String(currentRotatorPosition).padStart(3, '0'),
		String((currentRotatorPosition + rotatorStartPoint) % 360).padStart(3, '0'),
		String(targetA % 360).padStart(3, '0'),
		formatDir(diff),
		' should be ',
		formatDir(shouldBe),
		(diff === shouldBe ? 'OK' : '  '),
		' =>',
		String(currentRotatorPosition + diff).padStart(3, '0'),
	);
	return diff;
}

const rotatorStartPoint = 90;
const rotatorMotionRange = 450;
// let currentRotatorPosition = 180;
// position, target, shouldBe
//rotatorSetTargetAzimuth(180, 360, +90);



/*
sen = sensed position as returned by analogRead mapped to 0 - rotatorMotionRange
cur = current azimuth determined from the sensed position and initial offset (rotatorStartPoint)
tar = target azimuth
diff = the difference between the current azimuth and the target azimuth adjusted for the rotator end stops

The test cases below assume rotatorStartPoint of 090 and rotatorMotionRange of 450°.

pos az  az  -< >+
sen cur tar diff
*/
const testStrings = `
000 090 180 +090
180 270 180 -090
270 000 180 -180
360 090 180 +090
450 180 180 0

000 090 270 +180
180 270 270 0
270 000 270 -090
360 090 270 -180
450 180 270 -270

000 090 000 +270
180 270 000 +090
270 000 000 0
360 090 000 -090
450 180 000 -180
`.split("\n");
console.log("sen cur tar diff");
for (const testLine of testStrings) {
	if (testLine.trim().length === 0) {
		console.log();
		continue;
	}

	const parts = testLine.split(' ');
	rotatorSetTargetAzimuth(parseInt(parts[0]), parseInt(parts[2]), parseInt(parts[3]));
	// rotatorSetTargetAzimuth(360, 0, 0);
}