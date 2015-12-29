function on_launch()
{
	var width=400;
	var height=400;

	chrome.app.window.create('/window.html',
	{
		'innerBounds':
		{
			'width':width,'height':height,
			'maxWidth':width,'maxHeight':height,
			'minWidth':width,'minHeight':height
		}
	});
}

chrome.app.runtime.onLaunched.addListener(on_launch);
