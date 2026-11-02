import http from 'node:http';
import { verifyBarrier } from './index.js';

const PORT = process.env.SENTINEL_PORT || 8080;
const DLP_REGEX = /(PRIVATE_KEY|SECRET_KEY|sk-[a-zA-Z0-9]{20,}|ghp_[a-zA-Z0-9]{30,}|AIza[0-9A-Za-z-_]{35})/g;

for (let i = 0; i < 500; i++) DLP_REGEX.test('warmup');
verifyBarrier(new BigInt64Array([0x04a1f8e9c201b7a6n, 0n, 0n, 0n]), 0x04a1f8e9c201b7a6n);

const server = http.createServer((req, res) => {
    if (req.method !== 'POST' || req.url !== '/v1/inspect') {
        res.writeHead(404, { 'Content-Type': 'application/json' });
        return res.end(JSON.stringify({ error: 'Not Found' }));
    }

    let body = '';
    req.on('data', chunk => (body += chunk));
    req.on('end', () => {
        try {
            const t0 = performance.now();
            const data = JSON.parse(body);

            const hwAuthorized = verifyBarrier(
                new BigInt64Array([0x04a1f8e9c201b7a6n, 0n, 0n, 0n]),
                0x04a1f8e9c201b7a6n
            );
            const headerVerified = req.headers['x-zerolabz-pqzta-barrier'] === 'VERIFIED';
            const rootHash = req.headers['x-zerolabz-bundle-root'] || 'UNATTESTED';

            DLP_REGEX.lastIndex = 0;
            const hasLeak = DLP_REGEX.test(data.promptContext || '');
            const redactedContext = (data.promptContext || '').replace(
                DLP_REGEX,
                '[REDACTED_BY_ZEROLABZ_SENTINEL]'
            );
            const latencyUs = ((performance.now() - t0) * 1000).toFixed(2);

            const status = !hwAuthorized || !headerVerified
                ? 'BLOCKED_PQZTA_BARRIER'
                : hasLeak
                ? 'BLOCKED_DLP_VIOLATION'
                : 'ALLOWED';

            res.writeHead(status === 'ALLOWED' ? 200 : 403, { 'Content-Type': 'application/json' });
            res.end(
                JSON.stringify({
                    sentinel: 'zerolabz-sentinel-v1',
                    status,
                    dlpRedacted: hasLeak,
                    verifiedBundleRoot: rootHash,
                    manifoldDim: data?.activeFile?.manifold16d?.length ?? 0,
                    inspectionLatencyUs: latencyUs,
                    redactedPreview: hasLeak ? redactedContext.slice(0, 200) : undefined
                })
            );
        } catch (err) {
            res.writeHead(400, { 'Content-Type': 'application/json' });
            res.end(JSON.stringify({ error: 'Invalid payload' }));
        }
    });
});

server.listen(PORT, '127.0.0.1', () => {
    console.log(`[ZeroLabz Sentinel Bridge] Listening on http://127.0.0.1:${PORT}/v1/inspect (PQ-ZTA + DLP Active)`);
});