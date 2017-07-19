const {app, BrowserWindow} = require('electron');

// module.paths.push('/Users/haotianzhang/Project/CoreTrec/TrecCoreWeb/treccoreweb/static/js/');

let mainWindow;

app.on('window-all-closed', function() {
  app.quit();
});

app.on('ready', function() {
  mainWindow = new BrowserWindow({  frame: false, 
  	webPreferences:{
  			nodeIntegration:false
  		}
  	});
  mainWindow.setFullScreen(true);
  mainWindow.loadURL('http://ec2-54-218-106-163.us-west-2.compute.amazonaws.com:9000/');
  // mainWindow.loadURL('http://127.0.0.1:8000');
});
