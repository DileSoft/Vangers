const fs = require('fs');
const path = require('path');

/**
 * Конвертирует внутренний формат Vangers (H, W, Pixels) в стандартный Windows BMP (8-bit indexed).
 * @param {string} inputPath Путь к исходному файлу
 * @param {string} outputPath Путь к выходному BMP
 * @param {string} palPath Путь к файлу палитры (опционально)
 */
function convertVangersBmpToStandard(inputPath, outputPath, palPath) {
    const buffer = fs.readFileSync(inputPath);
    
    // Внутренний формат Vangers для iscreen (определено экспериментально):
    // uint16 width (2 bytes)
    // uint16 height (2 bytes)
    // raw pixel data (8-bit indexed)
    
    let offset = 0;
    const width = buffer.readUInt16LE(offset); offset += 2;
    const height = buffer.readUInt16LE(offset); offset += 2;
    
    console.log(`Converting ${inputPath}: ${width}x${height}`);
    
    // Проверка на корректность размера
    const pixelDataSize = width * height;
    if (buffer.length < offset + pixelDataSize) {
        console.warn(`Warning: File size (${buffer.length}) is smaller than expected (${offset + pixelDataSize})`);
    }

    const pixels = buffer.slice(offset, offset + pixelDataSize);
    
    // В BMP каждая строка должна быть выровнена по 4 байта
    const rowPadding = (4 - (width % 4)) % 4;
    const stride = width + rowPadding;
    const bmpPixelDataSize = stride * height;

    const bmpPixels = Buffer.alloc(bmpPixelDataSize);
    for (let y = 0; y < height; y++) {
        // Копируем строки. В BMP строки идут СНИЗУ ВВЕРХ, 
        // но так как мы используем отрицательную высоту в заголовке, 
        // мы можем писать их сверху вниз.
        const srcOffset = y * width;
        const destOffset = y * stride;
        pixels.copy(bmpPixels, destOffset, srcOffset, srcOffset + width);
    }
    
    // Создаем заголовок BMP (BITMAPFILEHEADER - 14 байт)
    const fileHeader = Buffer.alloc(14);
    const fileSize = 14 + 40 + (256 * 4) + bmpPixelDataSize; 
    fileHeader.write('BM', 0);
    fileHeader.writeUInt32LE(fileSize, 2);
    fileHeader.writeUInt32LE(14 + 40 + (256 * 4), 10); // Offset to pixel data
    
    // Создаем информационный заголовок (BITMAPINFOHEADER - 40 байт)
    const infoHeader = Buffer.alloc(40);
    infoHeader.writeUInt32LE(40, 0); // Header size
    infoHeader.writeInt32LE(width, 4);
    infoHeader.writeInt32LE(-height, 8); // Отрицательная высота
    infoHeader.writeUInt16LE(1, 12); // Planes
    infoHeader.writeUInt16LE(8, 14); // Bit count (8-bit)
    infoHeader.writeUInt32LE(0, 16); // Compression (none)
    infoHeader.writeUInt32LE(bmpPixelDataSize, 20); // Image size
    
    // Создаем палитру (256 цветов * 4 байта BGRA)
    const palette = Buffer.alloc(256 * 4);
    if (palPath && fs.existsSync(palPath)) {
        console.log(`Using palette: ${palPath}`);
        const palBuffer = fs.readFileSync(palPath);
        // PAL формат в Vangers: 768 байт (256 * 3 RGB)
        // Цвета часто хранятся в диапазоне 0-63, поэтому их нужно умножить на 4
        // Проверяем, если максимальное значение не превышает 63
        let needsScaling = true;
        for (let i = 0; i < Math.min(palBuffer.length, 768); i++) {
            if (palBuffer[i] > 63) {
                needsScaling = false;
                break;
            }
        }

        const scale = needsScaling ? 4 : 1;
        if (needsScaling) console.log('Scaling palette colors (0-63 -> 0-252)');

        for (let i = 0; i < 256; i++) {
            const r = palBuffer[i * 3] * scale;
            const g = palBuffer[i * 3 + 1] * scale;
            const b = palBuffer[i * 3 + 2] * scale;
            palette[i * 4] = b;     // B
            palette[i * 4 + 1] = g; // G
            palette[i * 4 + 2] = r; // R
            palette[i * 4 + 3] = 0; // Reserved
        }
    } else {
        console.log('No palette found, using grayscale.');
        for (let i = 0; i < 256; i++) {
            palette[i * 4] = i;     // B
            palette[i * 4 + 1] = i; // G
            palette[i * 4 + 2] = i; // R
            palette[i * 4 + 3] = 0; // Reserved
        }
    }
    
    // Собираем всё вместе
    const outputBuffer = Buffer.concat([fileHeader, infoHeader, palette, bmpPixels]);
    
    fs.writeFileSync(outputPath, outputBuffer);
    console.log(`Saved to ${outputPath}`);
}

// Запуск: node convert_bmp.js <input.bmp> [output.bmp] [input.pal]
const args = process.argv.slice(2);
if (args.length < 1) {
    console.log('Usage: node convert_bmp.js <input_vangers_bmp> [output_standard_bmp] [input_vangers_pal]');
    process.exit(1);
}

const input = args[0];
const output = args[1] || input.replace(/\.bmp$/i, '_converted.bmp');
let pal = args[2];

// Если палитра не указана, пробуем найти файл с тем же именем, но расширением .pal
if (!pal) {
    const potentialPal = input.replace(/\.bmp$/i, '.pal');
    if (fs.existsSync(potentialPal)) {
        pal = potentialPal;
    }
}

try {
    convertVangersBmpToStandard(input, output, pal);
} catch (err) {
    console.error('Error during conversion:', err.message);
}
