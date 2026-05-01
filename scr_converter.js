const fs = require('fs');

/**
 * Very basic parser for the Vangers .scr format.
 * It identifies blocks like Screen "Name" { ... } and Object "Name" { ... }
 * and key-value pairs or single-word commands.
 */
function parseScr(text) {
    const lines = text.split('\n');
    let root = { type: 'root', children: [] };
    let stack = [root];
    let current = root;

    const blockRegex = /^(Screen|Object|Element|Event|EvComm)\s*"?([^"{]*)"?\s*\{/i;
    const defineRegex = /^#define\s+(\w+)\s+(.+)$/;
    const includeRegex = /^#include\s+"(.+)"/;

    for (let line of lines) {
        line = line.trim();
        if (!line || line.startsWith('//')) continue;

        // Handle defines and includes as special top-level or inline items
        if (line.startsWith('#define')) {
            const match = line.match(defineRegex);
            if (match) {
                current.children.push({ type: 'define', name: match[1], value: match[2] });
            }
            continue;
        }
        if (line.startsWith('#include')) {
            const match = line.match(includeRegex);
            if (match) {
                current.children.push({ type: 'include', file: match[1] });
            }
            continue;
        }

        // Handle block start
        const blockMatch = line.match(blockRegex);
        if (blockMatch) {
            const newNode = {
                type: blockMatch[1].toLowerCase(),
                name: blockMatch[2].trim(),
                children: []
            };
            current.children.push(newNode);
            stack.push(newNode);
            current = newNode;
            continue;
        }

        // Handle block end
        if (line === '}') {
            stack.pop();
            current = stack[stack.length - 1];
            if (!current) {
                console.warn('Warning: Unmatched closing brace. Resetting to root.');
                current = root;
                stack = [root];
            }
            continue;
        }

        // Handle properties/commands
        // Split by whitespace but respect quotes if any (simple version)
        const parts = line.split(/\s+/);
        if (parts.length > 0 && current) {
            current.children.push({ type: 'property', key: parts[0], values: parts.slice(1) });
        }
    }

    return root;
}

/**
 * Converts the JSON structure back to the .scr format.
 */
function stringifyScr(node, indent = '') {
    let output = '';
    if (node.type === 'root') {
        return node.children.map(child => stringifyScr(child, '')).join('\n');
    }

    if (node.type === 'define') {
        return `#define ${node.name} ${node.value}`;
    }
    if (node.type === 'include') {
        return `#include "${node.file}"`;
    }
    if (node.type === 'property') {
        return `${indent}${node.key}\t${node.values.join(' ')}`;
    }

    // Generic block
    const typeLabel = node.type.charAt(0).toUpperCase() + node.type.slice(1);
    const nameLabel = node.name ? ` "${node.name}"` : '';
    output += `${indent}${typeLabel}${nameLabel}\n${indent}{\n`;
    output += node.children.map(child => stringifyScr(child, indent + '\t')).join('\n');
    output += `\n${indent}}`;

    return output;
}

// CLI usage
const args = process.argv.slice(2);
if (args.length < 3) {
    console.log('Usage: node scr_converter.js [to-json|from-json] input_file output_file');
    process.exit(1);
}

const mode = args[0];
const inputPath = args[1];
const outputPath = args[2];

const inputData = fs.readFileSync(inputPath, 'utf8');

if (mode === 'to-json') {
    const json = parseScr(inputData);
    fs.writeFileSync(outputPath, JSON.stringify(json, null, 2));
    console.log(`Converted ${inputPath} to JSON -> ${outputPath}`);
} else if (mode === 'from-json') {
    const json = JSON.parse(inputData);
    const scr = stringifyScr(json);
    fs.writeFileSync(outputPath, scr);
    console.log(`Converted ${inputPath} from JSON -> ${outputPath}`);
}
