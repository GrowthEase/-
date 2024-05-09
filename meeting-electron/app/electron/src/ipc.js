const { ipcRenderer } = require('electron');
const log = require('electron-log/renderer');
log.transports.console.format = '[{y}-{m}-{d} {h}:{i}:{s}.{ms}] {text}';

window.electronLog = log.info;

window.isElectronNative = true;
window.ipcRenderer = ipcRenderer;
window.platform = process.platform;
window.systemPlatform = process.platform;
window.isWins32 = process.platform === 'win32';
