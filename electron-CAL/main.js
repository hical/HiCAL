const {app, BrowserWindow} = require('electron');

let mainWindow;

app.on('window-all-closed', function() {
  app.quit();
});

app.on('ready', function() {
  mainWindow = new BrowserWindow({  frame: true, 
  	webPreferences:{
  			nodeIntegration:false
  		}
  	});
  mainWindow.setFullScreen(true);
  mainWindow.loadURL('http://ec2-54-218-106-163.us-west-2.compute.amazonaws.com:9000/');
});
