/*function gruveo_t(div)
{
	if(!div)
		return null;
	this.div=div;
	this.video_frame=document.createElement("webview");
	//this.video_frame.src="https://www.gruveo.com/embed/";
	//this.video_frame.src="https://www.youtube.com/v/zsqWawaOMCQ";
	this.video_frame.src="https://robotmoose.com/robots/";
	this.video_frame.style.width = this.video_frame.style.height = "480px";
	this.div.appendChild(this.video_frame);

}

gruveo_t.prototype.load=function()
{
	var myself=this;
	/*var req = new XMLHttpRequest();
	console.log(this.video_frame.src);
	req.open('GET',this.video_frame.src,true);
	req.onload = function (e)
	{
		myself.div.appendChild(myself.video_frame);
	};
	req.send();	

this.video_frame.addEventListener('permissionrequest',function(e)
{
	if (e.permission == 'media')
	{
		e.request.allow();
	}
});


}*/

/*function updateWebViews()
{
	var webview = document.getElementById("gruveo");
	console.log(webview);
	webview.style.height = document.documentElement.clientHeight + "px";
	webview.style.width = document.documentElement.clientWidth + "px";

}

onload = updateWebViews;
window.onresixe = updateWebViews;
*/

onload = function()
{
	var webview = document.querySelector('webview');
	console.log("onload called");
	webview.addEventListener('consolemessage', function(msg) {
		console.log(msg);
		});
	webview.addEventListener('permissionrequest', function(e) {
	  console.log("requesting permission" + e);
	  if (e.permission === 'media') {
	    e.request.allow();

	  }
	})
}

