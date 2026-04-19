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
        if (req.url.startsWith('/output')) {
          // Remove query string like ?t=1231235123 so fs.existsSync can find the file
          const urlPath = req.url.split('?')[0];
          
          // Use path.join to avoid root-resetting behavior if urlPath still has a leading slash
          const filePath = path.join(projectRoot, 'output', urlPath.replace('/output', ''));
          
          if (fs.existsSync(filePath)) {
            res.setHeader('Content-Type', 'application/json');
            res.setHeader('Cache-Control', 'no-store, no-cache, must-revalidate, proxy-revalidate');
            res.setHeader('Pragma', 'no-cache');
            res.setHeader('Expires', '0');
            const content = fs.readFileSync(filePath);
            return res.end(content);
          } else {
             // Return 404 JSON to prevent SPA fallback to index.html
             res.statusCode = 404;
             res.setHeader('Content-Type', 'application/json');
             return res.end(JSON.stringify({ error: 'File not found locally' }));
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

            // Execute the analyzer in headless mode (no interactive menu)
            exec('./master_bin target_input.c --headless', { cwd: projectRoot }, (error, stdout, stderr) => {
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
