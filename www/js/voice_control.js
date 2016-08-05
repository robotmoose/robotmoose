var make_lexer = function(states){
    return function(word){
        return states[word];
    }
};

function number_parser_t(){
    this.NUMBERS = {
        'zero'     :   0,
        'one'      :   1,
        'two'      :   2,
        'three'    :   3,
        'four'     :   4,
        'five'     :   5,
        'six'      :   6,
        'seven'    :   7,
        'eight'    :   8,
        'nine'     :   9,
        'ten'      :  10,
        'eleven'   :  11,
        'twelve'   :  12,
        'thirteen' :  13,
        'fourteen' :  14,
        'fifteen'  :  15,
        'sixteen'  :  16,
        'seventeen':  17,
        'eighteen' :  18,
        'nineteen' :  19,
        'twenty'   :  20,
        'thirty'   :  30,
        'fourty'   :  40,
        'fifty'    :  50,
        'sixty'    :  60,
        'seventy'  :  70,
        'eighty'   :  80,
        'ninty'    :  90,
        'hundred'  : 100,
        'thousand' : 1e3,
        'million'  : 1e6,
        'billion'  : 1e9,
        'trillion' :1e12
    };
    this.lexer = make_lexer(this.NUMBERS);
}

number_parser_t.prototype.parse = function(words){
    var stack = [];
    var stack_depth=0;
    var last_res = 0;
    for(var w of words){
        var res = this.lexer(w);
        if(res>=100){
            var tmp = 0;
            for(stack_depth;stack_depth>0;stack_depth--){
                tmp += stack.pop();
            }
            if(tmp==0) tmp=1;
            tmp *= res;
            stack.push(tmp);
            stack_depth = 1;
        }
        else{
            stack.push(res);
            if(last_res < 1000)
                stack_depth+=1;
        }
        last_res = res;
    }

    var total = 0;
    while(stack.length!=0){
        total += stack.pop();
    }

    return total;
}

//constant time lookup of a word
number_parser_t.prototype.is_number = function(word){
    return word in this.NUMBERS
}

function preprocessor_t(){
    this.number_parser = new number_parser_t();
}

preprocessor_t.prototype.preprocess = function(text){
    this.text = text.toLowerCase();
    this.text = this.text.replace('-',' ');
    this.text = this.text.split(' ');
    for(var i=0;i<this.text.length;++i){
        if(this.number_parser.is_number(this.text[i])){
            var j;
            for(j=i;j<this.text.length &&this.number_parser.is_number(this.text[j]); ++j){}
            this.text.splice(i, j-i,this.number_parser.parse(this.text.slice(i,j)));
        }
    }
    //console.log(this.text);
    return this.text.join(' ');
}

var pp = new preprocessor_t();

lexer_t.prototype.valid_tokens = new Set(['HEY', 'ROBOT_NICK', 'DIRECTION', 'ACTION', 'DISTANCE', 'UNIT']);

function lexer_t(text, opts){
    _this = this;
    this.preprocessor = new preprocessor_t();
    _this.text = this.preprocessor.preprocess(text);
    _this.text_idx = 0;
    _this.keywords = {};
    _this.tokens = [];
    _this.add_keywords(['hey'], 'HEY');
    _this.add_keywords([opts.robot_nick || 'robot'], 'ROBOT_NICK');
    _this.add_keywords(['left', 'right', 'forward', 'backwards'], 'DIRECTION');
    _this.add_keywords(['drive', 'turn'], 'ACTION');
    _this.add_keywords(['then'], 'THEN');
    _this.add_keywords(['and'], 'THEN');
    _this.add_keywords(['pi'], 'PI', function(lexeme){lexeme.text = Math.PI; return lexeme});
    _this.add_keywords(['tau'], 'TAU', function(lexeme){lexeme.text = Math.PI*2; return lexeme;});
    _this.add_keywords(['radians', 'degrees'], 'ANGLE_UNIT');
    _this.add_keywords(['over'], 'DIVIDE');

    _this.add_token([/[0-9]+(?:.[0-9]+)?/], 'NUMBER');
    _this.add_token([/(?:centi)?meters?/, /inch(?:es)?/, 'feet', 'foot'], "DISTANCE_UNIT", this.clean_distance_unit);
}

lexer_t.prototype.clean_distance_unit = function(lex){
    //remove the plural from the unit, just makes code more readable later
    //also, turn inches into inch
    if(lex.text == 'inches'){
        lex.text = 'inch';
    }
    if(lex.text == 'feet'){
        lex.text = 'foot';
    }
    if(lex.text.slice(-1) == 's'){
        lex.text = lex.text.slice(0,-1);
    }
    return lex;
}

lexer_t.prototype[Symbol.iterator] = function*(){
    while(!this.finished()){
        yield this.get_next();
    }
}

lexer_t.prototype.add_keywords = function(list, ret, callback){
    for(kw of list){
        this.keywords[kw] = {token_return:ret, callback:callback};
    }
}

lexer_t.prototype.add_token = function(list, ret, callback){
    for(var t of this.tokens){
        if(t.token_return == ret){
            for(i of list)
                t.expected_texts.push(i);
            return;
        }
    }
    this.tokens.push({expected_texts: list, token_return: ret, callback: callback});
}

lexer_t.prototype.get_text = function(){
    old_idx = this.text_idx;
    while(this.text[this.text_idx] != ' ' && this.text_idx < this.text.length) this.text_idx++;
    return this.text.slice(old_idx, this.text_idx);
}

lexer_t.prototype.get_next = function(){
    lexeme = {}
    
    lexeme.text = this.get_text();
    lexeme = this.get_token(lexeme);

    this.text_idx++;
    //console.log(lexeme);
    return lexeme;
}

lexer_t.prototype.get_token = function(lexeme){
    if(lexeme.text == '') return 'NULL';
    if(lexeme.text in this.keywords){
        lexeme.token = this.keywords[lexeme.text].token_return;
        if(this.keywords[lexeme.text].callback){ lexeme = this.keywords[lexeme.text].callback(lexeme)}
        return lexeme;
    }
    for(t of this.tokens){
        for(et of t.expected_texts){
            var res = lexeme.text.match(et);
            if(res && res[0].length == res.input.length){ 
                if(t.callback) lexeme = t.callback(lexeme);
                lexeme.token = t.token_return;
                return lexeme;
            }
        }
    }
    lexeme.token = 'ID';
    return lexeme;
}

lexer_t.prototype.finished = function(){
    return (this.text_idx >= this.text.length);
}

function parser_t(text, robot_name){
    this.robot_name = robot_name || "robot";
    this.lexer = new lexer_t(text, {robot_nick: robot_name});
    //console.log(this.lexer.tokens);
    this.lexeme = this.lexer.get_next();
    this.program = {};
    this.program.execute = false;
    this.program.text = '';
}

parser_t.prototype._next = function(){
    this.lexeme = this.lexer.get_next();
}

parser_t.prototype._accept = function(expected_token){
    if(this.lexeme.token == expected_token){
        var ret = this.lexeme.text;
        this._next();
        return ret;
    }
    return '';
}

parser_t.prototype._expect = function(expected_token){
    var tmp = this._accept(expected_token);
    if(tmp) return tmp;
    throw "Expected token not found: {expected token: " + expected_token + "," + " current token:" + this.lexeme.token + " }"
        + " current text: " + this.lexeme.text;
}

parser_t.prototype._accept_text = function(expected_text, expected_token){
    if(this.lexeme.token == expected_token && this.lexeme.text == expected_text){
        this._next();
        return true;
    }
    return false;
}

parser_t.prototype._expect_text = function(expected_text, expected_token){
    if(this._accept_text(expected_text, expected_token)) return true;
    throw "Unexpected token: {" + expected_text + "," + expected_token + "}";
    return false;
}

//returns the "compiled" js from english
parser_t.prototype.parse = function(){
    if(this.salutation())
        this.program.execute = true;
    while(!this.lexer.finished()){
        this.command();
        if(!this.lexer.finished()){
            this._expect("THEN");
            while(this._accept("THEN")){}
        }
    }
}

parser_t.prototype.salutation = function(){
    this._expect("HEY");
    this._expect("ROBOT_NICK");
    //get rid of that stupid human politness
    //who cares if its gramatical
    while(
        this._accept_text("can", "ID")  ||
        this._accept_text("could", "ID")|| 
        this._accept_text("will", "ID")|| 
        this._accept_text("you", "ID")  ||
        this._accept_text("please", "ID")){}
    return true;
}

parser_t.prototype.command = function(){
    this.action();
}

parser_t.prototype.action = function(){
    if(this._accept_text("drive", "ACTION")){
        this.drive_specs();
        return
    }
    else if(this._accept_text("turn", "ACTION")){
        this.turn_specs();
        return
    }
    throw "Did not find expected token: {" + this.lexeme.text + "}";
}

parser_t.prototype.drive_specs = function(){
    var lexeme_text;
    var drive_params = {};
    if(lexeme_text = this._accept("DIRECTION")){
        drive_params.direction = lexeme_text;
    }
    drive_params.distance = this.parse_distance();

    this.make_drive_command(drive_params);
}

parser_t.prototype.make_drive_command = function(drive_params){
    if(drive_params.distance){
        this.program.text += drive_params.direction + "(" + drive_params.distance + ");\n";
    }
    else{
        if(drive_params.direction != 'forward')
            this.program.text += drive_params.direction + "(90);\n";
        this.program.text += "drive(10,10);\n";
    }
}

parser_t.prototype.turn_specs = function(){
    var lexeme_text;
    var turn_params = {};
    if(lexeme_text = this._accept("DIRECTION")){
        turn_params.direction = lexeme_text;
    }
    turn_params.angle = this.parse_angle();
    this.make_turn_command(turn_params);
}

parser_t.prototype.make_turn_command = function(turn_params){
    if(turn_params.direction){
        this.program.text += turn_params.direction + "(";
        if(turn_params.angle){
            this.program.text += turn_params.angle + ");\n";
        }
        else{
            this.program.text += "90);\n";
        }
    }
    else if(turn_params.angle){
        this.program.text += "right("+turn_params.angle+")";
    }
}

parser_t.prototype.parse_expression = function(){
    return this.parse_term();
}
parser_t.prototype.parse_number = function(){
    var num = this._accept("NUMBER") ;
    while(tmp = this._accept("NUMBER")){
        num*=10;
        num+=tmp;
    }
    var pi_part = this._accept("PI") || this._accept("TAU") || 0;
    if(pi_part && num) num *= pi_part;
    else if(pi_part) num = pi_part;
    return num;
}

parser_t.prototype.parse_factor = function(){
    var num = this.parse_number();
    while(true){
        if(this._accept("DIVIDE")){
            num /= this.parse_number();
        }
        else if(this._accept("MULTIPLY")){
            num *= this.parse_number();
        }
        else{
            return num;
        }
    }
}

parser_t.prototype.parse_term = function(){
    var num = this.parse_factor();
    while(true){
        if(this._accept("ADD")){
            num += this.parse_factor();
        }
        else if(this._accept("SUBTRACT")){
            num -= this.parse_factor();
        }
        else{
            return num;
        }
    }
}

parser_t.prototype.parse_distance = function(){
    if(num = this.parse_expression()){
        var dist_unit = this._accept("DISTANCE_UNIT");
        if(dist_unit == 'meter'){
            num*=100;
        }
        else if(dist_unit == 'inch'){
            num*=2.54;
        }
        else if(dist_unit == 'foot'){
            num*=2.54*12.0;
        }
        console.log(num, dist_unit);
        return num;
    }
    else 
        return;
}

parser_t.prototype.parse_angle = function(){
    if(num = this.parse_expression()){
        var angle_unit = this._accept("ANGLE_UNIT");
        if(angle_unit == 'radians'){
            num = (num*180)/(Math.PI);
        }
    }
    return num;
}

parser_t.prototype.distance_convert = function(distance_text){
    //right now, its a dummy function
    //eventually, it has to convert to whatever the front end uses
    return distance_text;
}

parser_t.prototype.angle_convert = function(angle_text){
    //dummy function, needs to convert eventually
    return angle_text;
}

function voice_control_t(div, robot){
    //UI stuff
    this.div = div;

    this.table = new_table(this.div, 2,1);
    maximize(this.table);
    this.table.rows[1].style.height = "0%";

    this.transcription_text_box = document.createElement("textarea");
    this.transcription_text_box.className = "form-control";
    this.transcription_text_box.style.height = "100%"
    this.transcription_text_box.style.width = "100%"
    this.transcription_text_box.id = "transcription_text_box";
    this.table.rows[0].cells[0].appendChild(this.transcription_text_box);

    this.record_button = document.createElement("Button");
    this.record_button.className = "btn btn-primary";
    this.record_button.value = "Listen";
    this.record_button.id = "listen_button";
    this.record_button.textContent = "Listen";
    this.record_button.style.width = "100%";
    this.table.rows[1].cells[0].appendChild(this.record_button);

    //Voice stuff
    
    transcriber = new webkitSpeechRecognition();
    transcriber.continous = false;
    transcriber.intermresults = false;
    var recording = false;
    transcriber.onstart = function(){
        document.getElementById("listen_button").textContent = "Listening...";
    }
    transcriber.onerror = function(e){
        console.log(event)
    }
    transcriber.onend = function(){
        document.getElementById("listen_button").textContent = "Listen";
    }
    transcriber.onresult = function(event){
        console.log(event)
        document.getElementById("transcription_text_box").value = event.results[0][0].transcript;
        p = new parser_t(event.results[0][0].transcript, 'robot');
    }
    this.record_button.onclick = function(event){
        if(!recording){
            transcriber.start();
            recording = true;
        }
        else{
            transcriber.stop();
            recording = false;
        }
    }

}
