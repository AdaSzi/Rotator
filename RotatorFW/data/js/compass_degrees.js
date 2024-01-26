// Draws little lines every 5 degrees on the compass

const svg = document.querySelector('.compass__rose__dial');
const centerX = 65;
const centerY = 65;

for (let i = 0; i < 360; i += 5) {
    if (i % 90 !== 0) { // Exclude 0, 90, 180, and 270 degrees
        const line = document.createElementNS("http://www.w3.org/2000/svg", 'line');
        line.setAttribute('x1', centerX);
        line.setAttribute('y1', 9);
        line.setAttribute('x2', centerX);
        line.setAttribute('y2', 13);
        line.setAttribute('stroke', 'white');
        line.setAttribute('stroke-width', 1);
        line.setAttribute('transform', `rotate(${i} ${centerX} ${centerY})`);
        svg.appendChild(line);
    }
}
