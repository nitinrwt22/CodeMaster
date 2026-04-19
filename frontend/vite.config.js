import { defineConfig } from 'vite'
import react from '@vitejs/plugin-react'
import fs from 'fs'
import path from 'path'
import { fileURLToPath } from 'url'
import { exec } from 'child_process'

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const projectRoot = path.resolve(__dirname, '..');

function customMiddlewarePlugin() {
  return {
    name: 'custom-middleware',
    configureServer(server) {
      server.middlewares.use((req, res, next) => {
        // Handle /output static files
        if (req.url.startsWith('/output')) {
          const filePath = path.resolve(projectRoot, 'output', req.url.replace('/output', ''));
          // In simpler terms, if req.url is /output/symbols.json -> resolves to ../output/symbols.json
          if (fs.existsSync(filePath)) {
            res.setHeader('Content-Type', 'application/json');
            const content = fs.readFileSync(filePath);
            return res.end(content);
          }
        }
        
        // Handle /api/analyze endpoint
        if (req.url === '/api/analyze' && req.method === 'POST') {
          let body = '';
          req.on('data', chunk => {
            body += chunk.toString();
          });
          
          req.on('end', () => {
             // Parse body safely
            let sourceCode = '';
            try {
               const parsed = JSON.parse(body);
               sourceCode = parsed.source;
            } catch (e) {
               sourceCode = body; // fallback to raw string
            }

            const targetFilePath = path.resolve(projectRoot, 'target_input.c');
            fs.writeFileSync(targetFilePath, sourceCode);

            // Execute the analyzer (pipe newline since master_bin waits for ENTER)
            exec('echo "" | ./master_bin target_input.c', { cwd: projectRoot }, (error, stdout, stderr) => {
               if (error) {
                 console.error('Execution error:', error);
                 res.statusCode = 500;
                 return res.end(JSON.stringify({ error: error.message }));
               }
               res.setHeader('Content-Type', 'application/json');
               res.end(JSON.stringify({ success: true, message: 'Analysis complete' }));
            });
          });
          return; // Stop execution here
        }

        next();
      });
    }
  }
}

// https://vite.dev/config/
export default defineConfig({
  plugins: [react(), customMiddlewarePlugin()],
})
