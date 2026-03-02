// vite.config.js
import { resolve } from "path";
import { defineConfig } from "vite";

export default defineConfig({
  build: {
    assetsDir: '',
    rollupOptions: {
      input: {
        preseteditor: resolve(__dirname, "preseteditor.html"),
        // index: resolve(__dirname, "index.html"),
        // simulator: resolve(__dirname, "simulator.html"),
      },
    },
  },
});
