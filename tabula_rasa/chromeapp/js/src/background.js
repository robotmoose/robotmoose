var appWin = null;

function onWindowCreated()
{
	appWin = chrome.app.window.get('gruveo');//chrome.app.window.current();
	appWin.onBoundsChanged.addListener(onBoundsChanged);
	onBoundsChanged();

}

function onBoundsChanged()
{
	var webview = document.querySelector('webview');
	var bounds = win.getBounds();
	webview.style.height = bounds.height + 'px';
	webview.style.width = bounds.width + 'px';
	console.log("In resize");
}

function on_launch()
{
	var width=600;
	var height=400;

	chrome.app.window.create('/window.html',
	{
		'innerBounds':
		{
			'width':width,'height':height,
		//	'maxWidth':width,'maxHeight':height,
			'minWidth':width,'minHeight':height
		}
	});
	chrome.app.window.create('gruveo.html',
		{
			id:'gruveo'
		}
			//onWindowCreated
		);

}

chrome.app.runtime.onLaunched.addListener(on_launch);


