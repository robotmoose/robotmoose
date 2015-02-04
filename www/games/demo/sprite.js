function sprite_t(source,frames)
{
	var myself=this;

	myself.image=new Image();
	myself.image.src=source;
	myself.frame=0;
	myself.frame_count=frames;
	myself.x_scale=1;
	myself.y_scale=1;
	myself.width=myself.image.width/myself.frame_count;
	myself.height=myself.image.height;

	myself.draw=function(simulation)
	{
		if(myself.frame>=myself.frame_count)
			myself.frame=0;

		myself.width=myself.image.width/myself.frame_count;
		myself.height=myself.image.height;

		if(simulation)
		{
			simulation.ctx.save();
			simulation.ctx.translate(-(myself.width/2.0*myself.x_scale),-(myself.height/2.0*myself.y_scale));
			simulation.ctx.scale(myself.x_scale,myself.y_scale);

			simulation.ctx.drawImage(myself.image,
				Math.floor(myself.frame)*myself.width,
				0,
				myself.width,
				myself.height,
				0,
				0,
				myself.width,
				myself.height);

			simulation.ctx.restore();
		}
	};
};