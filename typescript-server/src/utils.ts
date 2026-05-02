export class XBuffer {
    private buffer: Buffer;
    private offset: number = 0;

    constructor(size: number = 1024) {
        this.buffer = Buffer.alloc(size);
    }

    get data() {
        return this.buffer.slice(0, this.offset);
    }

    writeUInt8(val: number) {
        this.buffer.writeUInt8(val, this.offset++);
    }

    writeInt16LE(val: number) {
        this.buffer.writeInt16LE(val, this.offset);
        this.offset += 2;
    }

    writeUInt32LE(val: number) {
        this.buffer.writeUInt32LE(val, this.offset);
        this.offset += 4;
    }

    writeInt32LE(val: number) {
        this.buffer.writeInt32LE(val, this.offset);
        this.offset += 4;
    }

    writeString(str: string) {
        const len = this.buffer.write(str, this.offset);
        this.offset += len;
        this.buffer.writeUInt8(0, this.offset++); // Null terminator
    }

    writeBytes(buf: Buffer) {
        buf.copy(this.buffer, this.offset);
        this.offset += buf.length;
    }

    // Pylance/C++ - like begin_event helper
    beginEvent(eventCode: number) {
        this.writeUInt8(eventCode);
    }
}

export class XReader {
    private offset: number = 0;
    constructor(private buffer: Buffer) {}

    get isEOF() {
        return this.offset >= this.buffer.length;
    }

    tell(): number {
        return this.offset;
    }

    readUInt8() {
        return this.buffer.readUInt8(this.offset++);
    }

    readInt16LE() {
        const val = this.buffer.readInt16LE(this.offset);
        this.offset += 2;
        return val;
    }

    readUInt16LE() {
        const val = this.buffer.readUInt16LE(this.offset);
        this.offset += 2;
        return val;
    }

    readUInt32LE() {
        const val = this.buffer.readUInt32LE(this.offset);
        this.offset += 4;
        return val;
    }

    readInt32LE() {
        const val = this.buffer.readInt32LE(this.offset);
        this.offset += 4;
        return val;
    }

    readString(): string {
        let end = this.offset;
        while (end < this.buffer.length && this.buffer[end] !== 0) {
            end++;
        }
        const str = this.buffer.toString('utf8', this.offset, end);
        this.offset = end + 1; // skip null terminator
        return str;
    }

    readBytes(n: number): Buffer {
        const slice = this.buffer.slice(this.offset, this.offset + n);
        this.offset += n;
        return slice;
    }
}
