const fs = require('fs');

/**
 * Advanced token-based AST parser for Vangers .scr files.
 * Preserves exact formatting, whitespace, and comments.
 */
function parseScr(text) {
    const root = { type: 'root', children: [] };
    const stack = [root];
    let current = root;

    const tokens = [
        { name: 'comment_block', regex: /^\/\*[\s\S]*?\*\// },
        { name: 'comment_inline', regex: /^\/\/.*/ },
        { name: 'preprocessor', regex: /^#(define|include)/ },
        { name: 'block_start', regex: /^(Screen|Object|Element|Event|EvComm)(\s+"[^"]*")?\s*\{/i },
        { name: 'block_end', regex: /^\}/ },
        { name: 'tab', regex: /^\t+/ },
        { name: 'space', regex: /^ +/ },
        { name: 'newline', regex: /^[\n\r]+/ },
        { name: 'string_quoted', regex: /^"[^"]*"/ },
        { name: 'constant', regex: /^\$[A-Za-z0-9_$]+/ },
        { name: 'word', regex: /^[^\s\n\r\{\}]+/ }
    ];

    let remaining = text;

    while (remaining.length > 0) {
        let matched = false;
        for (const token of tokens) {
            const match = remaining.match(token.regex);
            if (match) {
                const raw = match[0];
                const node = { type: token.name, raw };

                if (token.name === 'block_start') {
                    node.children = [];
                    current.children.push(node);
                    stack.push(node);
                    current = node;
                } else if (token.name === 'block_end') {
                    current.children.push(node);
                    stack.pop();
                    current = stack[stack.length - 1] || root;
                } else {
                    current.children.push(node);
                }

                remaining = remaining.slice(raw.length);
                matched = true;
                break;
            }
        }

        if (!matched) {
            current.children.push({ type: 'unknown', raw: remaining[0] });
            remaining = remaining.slice(1);
        }
    }

    return root;
}

function stringifyScr(node) {
    if (node.type === 'root' || node.children) {
        let out = (node.type === 'root') ? '' : node.raw;
        if (node.children) {
            out += node.children.map(stringifyScr).join('');
        }
        return out;
    }
    return node.raw || '';
}

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

