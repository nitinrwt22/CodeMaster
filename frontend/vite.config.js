import { defineConfig } from 'vite'
import react from '@vitejs/plugin-react'
import fs from 'fs'
import path from 'path'
import { fileURLToPath } from 'url'

const __dirname = path.dirname(fileURLToPath(import.meta.url));

function serveOutputPlugin() {
  return {
    name: 'serve-output',
    configureServer(server) {
      server.middlewares.use('/output', (req, res, next) => {
        const filePath = path.resolve(__dirname, '../output', `.${req.url}`);
        if (fs.existsSync(filePath)) {
          res.setHeader('Content-Type', 'application/json');
          const content = fs.readFileSync(filePath);
          res.end(content);
        } else {
          next();
        }
      });
    }
  }
}

// https://vite.dev/config/
export default defineConfig({
  plugins: [react(), serveOutputPlugin()],
})
