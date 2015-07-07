function TabView(div, name)
{
    if(!div || !name)
        return null;
    this.div = div;
    this.name = name.replace(/\s/g,"_");
    this.tabs = {};
    
    this.element = document.createElement("div");
    this.element.role = "tabpanel";
    this.div.appendChild(this.element);
    
    this.titles = document.createElement("ul");
    this.titles.className = "nav nav-tabs";
    this.titles.role = "tablist";
    this.element.appendChild(this.titles);
    
    this.contents = document.createElement("div");
    this.contents.className = "tab-content";
    this.element.appendChild(this.contents);
    
}
TabView.prototype.createTab=function(title)
{
    if(title)
    {
        if(!this.tabs[title])
        {
            this.tabs[title]={};
            
            this.tabs[title].li=document.createElement("li");
            this.tabs[title].li.role = "presentation";
            this.tabs[title].li.className = this.name + title;
            if(Object.keys(this.tabs).length == 1)
            {
                this.tabs[title].li.className += " active";
            }
    
            this.tabs[title].a=document.createElement("a");
            this.tabs[title].a.href= "#" + this.name + title;
            this.tabs[title].a.setAttribute("aria-controls", this.name + title);
            this.tabs[title].a.role= this.name + title;
            this.tabs[title].a.setAttribute("data-toggle", "tab");
            this.tabs[title].a.innerHTML = title;
    
            this.tabs[title].li.appendChild(this.tabs[title].a);
            this.titles.appendChild(this.tabs[title].li);
    
            this.tabs[title].contentDiv = document.createElement("div");
            this.tabs[title].contentDiv.role = "tabpanel";
            this.tabs[title].contentDiv.className = "tab-pane";
            this.tabs[title].contentDiv.id = this.name + title;
            if(Object.keys(this.tabs).length == 1)
            {
                this.tabs[title].contentDiv.className += " active";
            }
            this.contents.appendChild(this.tabs[title].contentDiv);

            this.tabs[title].div = document.createElement("div");
            this.tabs[title].contentDiv.appendChild(this.tabs[title].div);
    
        }
    }
}

TabView.prototype.removeTab=function(title)
{
    if(title)
    {
        if(this.tabs[title])
        {
            this.titles.removeChild(this.tabs[title].li);
            this.contents.removeChild(this.tabs[title].contentDiv);
            this.tabs[title] = null;
        }
    }
}

TabView.prototype.getTab=function(title)
{
    if(!title)
    {
        return null;
    }
    if(!this.tabs[title])
    {
        this.createTab(title);
        
    }
    return this.tabs[title].div;

}