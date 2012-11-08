

// *********************************************************************
// Utility functions
// *********************************************************************


function getURLParameter (name) {
    // http://stackoverflow.com/questions/1403888/get-url-parameter-with-jquery
    return decodeURI(
        (RegExp(name + '=' + '(.+?)(&|$)').exec(location.search)||[,null])[1]
    );
}


function int (s) {
    return parseInt(s);
}

function double (s) {
    return parseFloat(s);
}

// *********************************************************************
// Global variables
// *********************************************************************

var randomNumber = 1;

// Canvas element
var canvas = document.getElementById('myCanvas');

// Game state

var dataLoaded = false;
var gameLoaded = false;
var gamePaused = true;
var gamePreview = false;
var gameDirection = 1;          // jpetit: -1 does not work
var gameAnim = true;

var speed = 200;
var actRound = 0;
var FRAME_TIME = 1000/speed;
var frames = 0;
var FRAMES_PER_TURN = 4;

//Object for storing all the game data
var data = new Object();

//String for storing the raw data
var raw_data_str;

// Assigment of colors
var colors = new Array();
colors[0] = "0000ea"; // blue

//colors[1] = "39f21b"; // green
colors[1] = "030001" // black

colors[2] = "de1818"; // red

//colors[3] = "39f2d5"; // cyan
//colors[3] = "efbb34"; // golden 
colors[3] = "b507f5" // violet

function loadGame (game) {
    var xmlhttp;

    if (window.XMLHttpRequest) {
        // code for IE7+, Firefox, Chrome, Opera, Safari
        xmlhttp = new XMLHttpRequest();
    } else {
        // code for IE6, IE5
        xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");
    }
    xmlhttp.onreadystatechange = function() {
        if (xmlhttp.readyState==4) {
            raw_data_str = xmlhttp.responseText;
            dataLoaded = true;
        }
    }

    xmlhttp.open("GET", game, false);
    xmlhttp.send();
}


// parsing and initializing the game

function initGame () {

    // convert text to tokens

    var st = raw_data_str + "";
    var t = st.replace('\n', ' ').split(/\s+/);
    var p = 0;


    data.tileSize = 50;

    // read prelude

    s = t[p++];
    data.version = t[p++];
    if (s != "boladedrac") alert("Error at round " + round + ".\n" + "boladedrac");
    if (data.version != "v1") alert("Error at round " + round + ".\n" + "version");

    s = t[p++];
    data.nb_players = int(t[p++]);
    if (s != "nb_players") alert("Error at round " + round + ".\n" + "nb_players");

    s = t[p++];
    data.nb_rounds = int(t[p++]);
    if (s != "nb_rounds") alert("Error at round " + round + ".\n" + "nb_rounds");

    s = t[p++];
    data.nb_capsules = int(t[p++]);
    if (s != "nb_capsules") alert("Error at round " + round + ".\n" + "capsules");

    s = t[p++];
    data.nb_balls = int(t[p++]);
    if (s != "nb_balls") alert("Error at round " + round + ".\n" + "balls");

    s = t[p++];
    data.nb_beans = int(t[p++]);
    if (s != "nb_beans") alert("Error at round " + round + ".\n" + "beans");

    s = t[p++];
    data.nb_kintons = int(t[p++]);
    if (s != "nb_kintons") alert("Error at round " + round + ".\n" + "kintons");

    s = t[p++];
    data.goku_regen_time = int(t[p++]);
    if (s != "goku_regen_time") alert("Error at round " + round + ".\n" + "goku_regen_time");

    s = t[p++];
    data.bean_regen_time = int(t[p++]);
    if (s != "bean_regen_time") alert("Error at round " + round + ".\n" + "bean_regen_time");

    s = t[p++];
    data.kinton_regen_time = int(t[p++]);
    if (s != "kinton_regen_time") alert("Error at round " + round + ".\n" + "kinton_regen_time");

    s = t[p++];
    data.kinton_life_time = int(t[p++]);
    if (s != "kinton_life_time") alert("Error at round " + round + ".\n" + "kinton_life_time");

    s = t[p++];
    data.max_strength = int(t[p++]);
    if (s != "max_strength") alert("Error at round " + round + ".\n" + "max_strength");

    s = t[p++];
    data.res_strength = int(t[p++]);
    if (s != "res_strength") alert("Error at round " + round + ".\n" + "res_strength");

    s = t[p++];
    data.moving_penalty = int(t[p++]);
    if (s != "moving_penalty") alert("Error at round " + round + ".\n" + "moving_penalty");

    s = t[p++];
    data.kamehame_penalty = int(t[p++]);
    if (s != "kamehame_penalty") alert("Error at round " + round + ".\n" + "kamehame_penalty");

    s = t[p++];
    data.combat_penalty = int(t[p++]);
    if (s != "combat_penalty") alert("Error at round " + round + ".\n" + "combat_penalty");

    s = t[p++];
    data.rows = int(t[p++]);
    if (s != "rows") alert("Error at round " + round + ".\n" + "rows");

    s = t[p++];
    data.cols = int(t[p++]);
    if (s != "cols") alert("Error at round " + round + ".\n" + "cols");

    data.goku = new Array();
    for (var i = 0; i < data.nb_players; ++i) {
        data.goku[i] = new Object();
    }

    s = t[p++];
    if (s != "names") alert("Error at round " + round + ".\n" + "names");
    data.names = new Array();
    for (var i = 0; i < data.nb_players; ++i) {
        data.names[i] = t[p++];
    }

    data.rounds = new Array();
    for (var round = 0; round < data.nb_rounds; ++round) {

        $("#debug").html(round);

        if (t[p++] != "round") alert("Error at round " + round + ".\n" + "round");
        if (int(t[p++]) != round) alert("Error at round " + round + ".\n" + "wrong round");

        // maze
        data.rounds[round] = new Object();
        data.rounds[round].rows = new Array();
        for (var i = 0; i < data.rows; ++i)
            data.rounds[round].rows[i] = t[p++];

        // beans
        if (t[p++] != "beans") alert("Error at round " + round + ".\n" + "beans");
	data.rounds[round].beans = new Array();
        for (var k = 0; k < data.nb_beans; ++k) {
	    data.rounds[round].beans[k] = new Object();
            data.rounds[round].beans[k].pos_x   = int(t[p++]);
            data.rounds[round].beans[k].pos_y   = int(t[p++]);
            data.rounds[round].beans[k].present =     t[p++];
            data.rounds[round].beans[k].time    = int(t[p++]);
        }

        // kintons
        if (t[p++] != "kintons") alert("Error at round " + round + ".\n" + "kintons");
	data.rounds[round].kintons = new Array();
        for (var k = 0; k < data.nb_kintons; ++k) {
	    data.rounds[round].kintons[k] = new Object();
            data.rounds[round].kintons[k].pos_x   = int(t[p++]);
            data.rounds[round].kintons[k].pos_y   = int(t[p++]);
            data.rounds[round].kintons[k].present =     t[p++];
            data.rounds[round].kintons[k].time    = int(t[p++]);
        }

        // gokus
        data.rounds[round].team = new Array();
        data.rounds[round].cpu  = new Array();
        for (var i = 0; i < data.nb_players; ++i) {
            s = t[p++];
            if (s != "normal"    &&
		s != "on_kinton" &&
		s != "with_ball" &&
		s != "on_kinton_with_ball") alert("Error at round " + round + ".\n" + "goku");

            data.rounds[round].team[i] = new Object();
            data.rounds[round].team[i].pos_x    =    int(t[p++]);
            data.rounds[round].team[i].pos_y    =    int(t[p++]);
            data.rounds[round].team[i].time     =    int(t[p++]);
            data.rounds[round].team[i].state    =        t[p++];
            data.rounds[round].team[i].balls    =    int(t[p++]);
            data.rounds[round].team[i].strength =    int(t[p++]);
            data.rounds[round].team[i].kinton   =    int(t[p++]);
            data.rounds[round].cpu[i]           = double(t[p++]);

            if (s == "on_kinton" || s == "on_kinton_with_ball") data.rounds[round].team[i].on_kinton = true;
	    else                                                data.rounds[round].team[i].on_kinton = false;

            if (s == "with_ball" || s == "on_kinton_with_ball") data.rounds[round].team[i].with_ball = true;
	    else                                                data.rounds[round].team[i].with_ball = false;
	}

        if (round != data.nb_rounds - 1) {
            // actions asked
            if (t[p++] != "actions_asked") alert("Error at round " + round + ".\n" + "no actions_asked");
            for (var i = 0; i < data.nb_players; ++i) {
                if (int(t[p++]) != i) alert("Error at round " + round + ".\n" + "wrong player");
		// ignore
                t[p++];
                t[p++];
            }

            // actions done
            if (t[p++] != "actions_done") alert("Error at round " + round + ".\n" + "no actions_done");
            for (var i = 0; i < data.nb_players; ++i) {
                if (int(t[p++]) != i) alert("Error at round " + round + ".\n" + "wrong player");
                data.rounds[round].team[i].action_type = t[p++];
                data.rounds[round].team[i].action_dir  = t[p++];
            }
	}
    }

    // load tiles and sprites
    preloadImages();

    // prepare state variables
    gameLoaded = true;
    if (getURLParameter("start") == "yes") gamePaused = false;
    else gamePaused = true;

    gamePreview = true;

    // slider init
    $("#slider").slider( "option", "min", 0 );
    $("#slider").slider( "option", "max", data.nb_rounds );

    // turn things on
    $("#loading").hide();
    $("#canvas").show();
    $("#scoreboard").show();
}


function preloadImages () {

    data.img = new Array();

    // grass
    data.img.tile_grass0 = new Image();
    data.img.tile_grass0.src = "img/tile_grass0.png";
    data.img.tile_grass1 = new Image();
    data.img.tile_grass1.src = "img/tile_grass1.png";
    data.img.tile_grass2 = new Image();
    data.img.tile_grass2.src = "img/tile_grass2.png";
    data.img.tile_grass3 = new Image();
    data.img.tile_grass3.src = "img/tile_grass3.png";
    data.img.tile_grass4 = new Image();
    data.img.tile_grass4.src = "img/tile_grass4.png";

    // floor
    data.img.tile_floor = new Image();
    data.img.tile_floor.src = "img/tile_floor.png";

    // rocks
    data.img.tile_rock = new Image();
    data.img.tile_rock.src = "img/tile_rock.png";
    data.img.tile_rock1 = new Image();
    data.img.tile_rock1.src = "img/tile_rock1.png";
    data.img.tile_rock2 = new Image();
    data.img.tile_rock2.src = "img/tile_rock2.png";
    data.img.tile_rock3 = new Image();
    data.img.tile_rock3.src = "img/tile_rock3.png";
    data.img.tile_rock4 = new Image();
    data.img.tile_rock4.src = "img/tile_rock4.png";
    data.img.tile_rock5 = new Image();
    data.img.tile_rock5.src = "img/tile_rock5.png";
    data.img.tile_rock6 = new Image();
    data.img.tile_rock6.src = "img/tile_rock6.png";
    data.img.tile_rock7 = new Image();
    data.img.tile_rock7.src = "img/tile_rock7.png";
    data.img.tile_rock8 = new Image();
    data.img.tile_rock8.src = "img/tile_rock8.png";
    data.img.tile_rock9 = new Image();
    data.img.tile_rock9.src = "img/tile_rock9.png";
    data.img.tile_rock10 = new Image();
    data.img.tile_rock10.src = "img/tile_rock10.png";
    data.img.tile_rock11 = new Image();
    data.img.tile_rock11.src = "img/tile_rock11.png";
    data.img.tile_rock12 = new Image();
    data.img.tile_rock12.src = "img/tile_rock12.png";
    data.img.tile_rock13 = new Image();
    data.img.tile_rock13.src = "img/tile_rock13.png";
    data.img.tile_rock14 = new Image();
    data.img.tile_rock14.src = "img/tile_rock14.png";
    data.img.tile_rock15 = new Image();
    data.img.tile_rock15.src = "img/tile_rock15.png";
    data.img.tile_rock16 = new Image();
    data.img.tile_rock16.src = "img/tile_rock16.png";

    // objects
    data.img.tile_bean = new Image();
    data.img.tile_bean.src = "img/bean.png";
    data.img.tile_capsule = new Image();
    data.img.tile_capsule.src = "img/capsule.png";
    data.img.tile_ball = new Image();
    data.img.tile_ball.src = "img/ball.png";
    data.img.tile_kinton = new Image();
    data.img.tile_kinton.src = "img/kinton.png";

    // gokus without kinton
    data.img.spr_goku = new Array();
    for (var i = 0; i < data.nb_players; ++i) {
        data.img.spr_goku[i] = new Image();
        data.img.spr_goku[i].src = "img/goku-"+colors[i]+".png";
    }

    // gokus with kinton
    data.img.spr_kinton = new Array();
    data.img.spr_kinton = new Array();
    for (var i = 0; i < data.nb_players; ++i) {
        data.img.spr_kinton[i] = new Image();
        data.img.spr_kinton[i].src = "img/goku-kinton-"+colors[i]+".png";
    }

    // kamehame
    data.img.spr_kamehame_ini_b2t = new Image();
    data.img.spr_kamehame_ini_b2t.src = "img/kamehame-ini-b2t.png";
    data.img.spr_kamehame_ini_t2b = new Image();
    data.img.spr_kamehame_ini_t2b.src = "img/kamehame-ini-t2b.png";
    data.img.spr_kamehame_ini_l2r = new Image();
    data.img.spr_kamehame_ini_l2r.src = "img/kamehame-ini-l2r.png";
    data.img.spr_kamehame_ini_r2l = new Image();
    data.img.spr_kamehame_ini_r2l.src = "img/kamehame-ini-r2l.png";

    data.img.spr_kamehame_fin_b2t = new Image();
    data.img.spr_kamehame_fin_b2t.src = "img/kamehame-fin-b2t.png";
    data.img.spr_kamehame_fin_t2b = new Image();
    data.img.spr_kamehame_fin_t2b.src = "img/kamehame-fin-t2b.png";
    data.img.spr_kamehame_fin_l2r = new Image();
    data.img.spr_kamehame_fin_l2r.src = "img/kamehame-fin-l2r.png";
    data.img.spr_kamehame_fin_r2l = new Image();
    data.img.spr_kamehame_fin_r2l.src = "img/kamehame-fin-r2l.png";

    data.img.spr_kamehame_v = new Image();
    data.img.spr_kamehame_v.src = "img/kamehame-v.png";
    data.img.spr_kamehame_h = new Image();
    data.img.spr_kamehame_h.src = "img/kamehame-h.png";
}


function updateGame () {
    $("#slider").slider("option", "value", actRound);
}


function drawGame () {

    if (canvas.getContext) {
        var context = canvas.getContext('2d');
        var rectSize = data.tileSize;

	var fixedMargin = 10;
	var heightScore = 180;

	canvas.width  = window.innerWidth  - 2*fixedMargin;
        canvas.height = window.innerHeight - 2*fixedMargin - heightScore;

	var sw = canvas.width  /(rectSize*data.cols);
	var sh = canvas.height/(rectSize*data.rows);
	var s;
	if (sw < sh) {
	    s = sw;
	    var offset = (canvas.height - s*rectSize*data.rows)/ 2;
	    canvas.style.marginTop  = fixedMargin + offset;
	    canvas.style.marginLeft = fixedMargin;
	}
	else {
	    s = sh;
	    var offset = (canvas.width - s*rectSize*data.cols)/ 2;
	    canvas.style.marginTop  = fixedMargin;
	    canvas.style.marginLeft = fixedMargin + offset;
	}
        context.scale(s, s);

        // outer rectangle
        context.fillStyle = "rgb(0,0,0)";
        context.fillRect(0, 0, rectSize*data.cols, rectSize*data.rows);

	randomNumber = 1;
        // draw maze
        for (var i = 0; i < data.rows; i++)
            drawRow(actRound, i);

	// draw beans and kintons
        for (var k = 0; k < data.nb_beans; ++k)
	    if (data.rounds[actRound].beans[k].present == 'y') {
                var x = data.rounds[actRound].beans[k].pos_x;
                var y = data.rounds[actRound].beans[k].pos_y;
                var img = data.img.tile_bean;
                var ctx = canvas.getContext('2d');
                ctx.drawImage(img, y*data.tileSize, x*data.tileSize);
	    }
        for (var k = 0; k < data.nb_kintons; ++k)
	    if (data.rounds[actRound].kintons[k].present == 'y') {
                var x = data.rounds[actRound].kintons[k].pos_x;
                var y = data.rounds[actRound].kintons[k].pos_y;
                var img = data.img.tile_kinton;
                var ctx = canvas.getContext('2d');
                ctx.drawImage(img, y*data.tileSize, x*data.tileSize);
	    }

        // draw gokus
	var kamehame_pos_x = -1;
	var kamehame_pos_y = -1;
	var kamehame_dir = 'n';

        for (var i = 0; i < data.nb_players; i++) {

            if (data.rounds[actRound].team[i].state == 'a') {

                if (! gameAnim) {
                    var x = data.rounds[actRound].team[i].pos_x;
                    var y = data.rounds[actRound].team[i].pos_y;
                    var ctx = canvas.getContext('2d');
		    if (data.rounds[actRound].team[i].on_kinton) {
			var img = data.img.spr_kinton[i];
			ctx.drawImage(img, y*data.tileSize, x*data.tileSize);
		    }
		    else {
			var img = data.img.spr_goku[i];
			ctx.drawImage(img, y*data.tileSize, x*data.tileSize);
		    }
                } else {
		    if (data.rounds[actRound].team[i].action_type == 't') {
			kamehame_pos_x = data.rounds[actRound].team[i].pos_x;
			kamehame_pos_y = data.rounds[actRound].team[i].pos_y;
			kamehame_dir   = data.rounds[actRound].team[i].action_dir;
		    }
                    var x;
		    if (data.rounds[actRound].team[i].on_kinton) {
			x = data.rounds[actRound].team[i].pos_x;
			x = x*data.tileSize;
			if (data.rounds[actRound].team[i].action_type !='t')
                            x = setMovedFastX(x, data.rounds[actRound].team[i].action_dir);
		    }
		    else {
			var tmpRound = actRound;            // gokus without kinton can only move in even rounds
			if (actRound % 2 == 1) --tmpRound;
			x = data.rounds[tmpRound].team[i].pos_x;
			x = x*data.tileSize;
			if (data.rounds[tmpRound].team[i].action_type !='t')
			    x = setMovedX(x, data.rounds[tmpRound].team[i].action_dir);
		    }

                    var y;
		    if (data.rounds[actRound].team[i].on_kinton) {
			y = data.rounds[actRound].team[i].pos_y;
			y = y*data.tileSize;
			if (data.rounds[actRound].team[i].action_type !='t')
                            y = setMovedFastY(y, data.rounds[actRound].team[i].action_dir);
		    }
		    else {
			var tmpRound = actRound;            // gokus without kinton can only move in even rounds
			if (actRound % 2 == 1) --tmpRound;
			y = data.rounds[tmpRound].team[i].pos_y;
			y = y*data.tileSize;
			if (data.rounds[tmpRound].team[i].action_type !='t')
			    y = setMovedY(y, data.rounds[tmpRound].team[i].action_dir);
		    }

                    var ctx = canvas.getContext('2d');

		    // gokus with ball are surrounded by a small thick circle
                    if (data.rounds[actRound].team[i].with_ball) {
			ctx.beginPath();
			ctx.strokeStyle = colors[i];
			ctx.lineWidth = 5;
			ctx.arc(y+25, x+25, 32, 0, Math.PI*2, true);
			ctx.closePath();
			ctx.stroke();
                    }

		    // regenerated gokus are surrounded by a big thin circle
                    if (actRound > 6 && data.rounds[actRound - 6].team[i].state == 'd') {
                        ctx.beginPath();
                        ctx.strokeStyle = colors[i];
                        ctx.lineWidth = 2;
                        ctx.arc(y+25, x+25, 64, 0, Math.PI*2, true);
                        ctx.closePath();
                        ctx.stroke();
                    }

		    if (data.rounds[actRound].team[i].on_kinton) {
			var img = data.img.spr_kinton[i];
			ctx.drawImage(img, y, x);
		    }
		    else {
			var img = data.img.spr_goku[i];
			ctx.drawImage(img, y, x);
		    }
                }
            } else {

		// regenerated gokus are surrounded by a big thin circle
                if (actRound < data.nb_rounds - 6 && data.rounds[actRound + 6].team[i].state == 'a') {

                    var r = 6;
                    while (data.rounds[actRound + r].team[i].state != 'd') r--;
                    r++;

                    var x = data.rounds[actRound + r].team[i].pos_x;
                    x = x*data.tileSize;

                    var y = data.rounds[actRound + r].team[i].pos_y;
                    y = y*data.tileSize;

                    var ctx = canvas.getContext('2d');

                    ctx.beginPath();
                    ctx.strokeStyle = colors[i];
                    ctx.lineWidth = 2;
                    ctx.arc(y+25, x+25, 64-4*r, 0, Math.PI*2, true);
                    ctx.closePath();
                    ctx.stroke();
                }
            }
        }
	// draw kamehames
       if (frames < (FRAMES_PER_TURN/2))
	    drawKamehame(kamehame_pos_x, kamehame_pos_y, kamehame_dir);
    }
}

function drawKamehame(pos_x, pos_y, dir) {
    var x = pos_x;
    var y = pos_y;

    if      (dir == 't') --x;
    else if (dir == 'b') ++x;
    else if (dir == 'l') --y;
    else if (dir == 'r') ++y;

    // do not show kamehames that are degenerate (== there is no room)
    if (data.rounds[actRound].rows[x][y] == 'X') return;

    var ctx = canvas.getContext('2d');


    // paint initial part of kamehame
    if      (dir == 't') img = data.img.spr_kamehame_ini_b2t;
    else if (dir == 'b') img = data.img.spr_kamehame_ini_t2b;
    else if (dir == 'l') img = data.img.spr_kamehame_ini_r2l;
    else if (dir == 'r') img = data.img.spr_kamehame_ini_l2r;
    ctx.drawImage(img, y*data.tileSize, x*data.tileSize);

    if      (dir == 't') --x;
    else if (dir == 'b') ++x;
    else if (dir == 'l') --y;
    else if (dir == 'r') ++y;

    // using that the board is surrounded by rocks
    while (data.rounds[actRound].rows[x][y] != 'X') {

	// paint middle part of kamehame
	if (dir == 't' || dir == 'b') img = data.img.spr_kamehame_v;
	else                          img = data.img.spr_kamehame_h;
	ctx.drawImage(img, y*data.tileSize, x*data.tileSize);

	if      (dir == 't') --x;
	else if (dir == 'b') ++x;
	else if (dir == 'l') --y;
	else if (dir == 'r') ++y;
    }

    // paint final part of kamehame
    if      (dir == 't') img = data.img.spr_kamehame_fin_b2t;
    else if (dir == 'b') img = data.img.spr_kamehame_fin_t2b;
    else if (dir == 'l') img = data.img.spr_kamehame_fin_r2l;
    else if (dir == 'r') img = data.img.spr_kamehame_fin_l2r;
    ctx.drawImage(img, y*data.tileSize, x*data.tileSize);
}

//Note: X and Y of the next functions are reversed
function setMovedX (x, action) {
    if (action == 'n') return x;
    if (action == 't') {
        return (x - (((frames + FRAMES_PER_TURN * (actRound % 2))*data.tileSize)/(2*FRAMES_PER_TURN)));
    }
    if (action == 'b') {
        return (x + (((frames + FRAMES_PER_TURN * (actRound % 2))*data.tileSize)/(2*FRAMES_PER_TURN)));
    }
    return x;
}

function setMovedY (y, action) {
    if (action == 'n') return y;
    if (action == 'l')
        return (y - (((frames + FRAMES_PER_TURN * (actRound % 2))*data.tileSize)/(2*FRAMES_PER_TURN)));
    if (action == 'r')
        return (y + (((frames + FRAMES_PER_TURN * (actRound % 2))*data.tileSize)/(2*FRAMES_PER_TURN)));
    return y;
}

function setMovedFastX (x, action) {
    if (action == 'n') return x;
    if (action == 't') {
        return (x - ((frames*data.tileSize)/FRAMES_PER_TURN));
    }
    if (action == 'b') {
        return (x + ((frames*data.tileSize)/FRAMES_PER_TURN));
    }
    return x;
}

function setMovedFastY (y, action) {
    if (action == 'n') return y;
    if (action == 'l')
        return (y - ((frames*data.tileSize)/FRAMES_PER_TURN));
    if (action == 'r')
        return (y + ((frames*data.tileSize)/FRAMES_PER_TURN));
    return y;
}

function drawRow (round, row) {
    var ctx = canvas.getContext('2d');
    var rectSize = data.tileSize;
    for (var i = 0; i < data.cols; ++i) {
	randomNumber = (125 * randomNumber + 1) % 4096;
	switch (randomNumber % 5) {
	case 0: var img = data.img.tile_grass0; ctx.drawImage(img, i*rectSize, row*rectSize); break;
	case 1: var img = data.img.tile_grass1; ctx.drawImage(img, i*rectSize, row*rectSize); break;
	case 2: var img = data.img.tile_grass2; ctx.drawImage(img, i*rectSize, row*rectSize); break;
	case 3: var img = data.img.tile_grass3; ctx.drawImage(img, i*rectSize, row*rectSize); break;
	case 4: var img = data.img.tile_grass4; ctx.drawImage(img, i*rectSize, row*rectSize); break;
	}
        var type = data.rounds[round].rows[row][i];
        var img_tile = selectTile(round, type, row, i);
        ctx.drawImage(img_tile, i*rectSize, row*rectSize);
    }
}


function selectTile (round, type, row, col) {
    switch (type) {
    case 'X': //Rock
        return selectRock(round, type, row, col);
        break;
    case '.': //Empty
        return data.img.tile_floor;
        break;
    case 'C': //Capsule
        return data.img.tile_capsule;
        break;
    case 'B': //Ball
        return data.img.tile_ball;
        break;
    default:
        break;
    }
}


function selectRock (round, type, row, col) {
    var n = 0;
    var s = 0;
    var e = 0;
    var w = 0;

    if ((row-1) < 0) n = 0;
    else if (data.rounds[round].rows[row-1][col] == 'X') n = 1;
    if ((row+1) >= data.rows) s = 0;
    else if (data.rounds[round].rows[row+1][col] == 'X') s = 1;
    if ((col-1) < 0) e = 0;
    else if (data.rounds[round].rows[row][col-1] == 'X') e = 1;
    if ((col+1) >= data.cols) w = 0;
    else if (data.rounds[round].rows[row][col+1] == 'X') w = 1;

    if (n == 0 && s == 0 && w == 1 && e == 0) return data.img.tile_rock1;
    if (n == 0 && s == 0 && w == 1 && e == 1) return data.img.tile_rock2;
    if (n == 0 && s == 0 && w == 0 && e == 1) return data.img.tile_rock3;
    if (n == 1 && s == 0 && w == 0 && e == 0) return data.img.tile_rock4;
    if (n == 1 && s == 1 && w == 0 && e == 0) return data.img.tile_rock5;
    if (n == 0 && s == 1 && w == 0 && e == 0) return data.img.tile_rock6;
    if (n == 1 && s == 0 && w == 1 && e == 0) return data.img.tile_rock7;
    if (n == 1 && s == 0 && w == 0 && e == 1) return data.img.tile_rock8;
    if (n == 0 && s == 1 && w == 1 && e == 0) return data.img.tile_rock9;
    if (n == 0 && s == 1 && w == 0 && e == 1) return data.img.tile_rock10;
    if (n == 1 && s == 1 && w == 0 && e == 1) return data.img.tile_rock11;
    if (n == 1 && s == 0 && w == 1 && e == 1) return data.img.tile_rock12;
    if (n == 1 && s == 1 && w == 1 && e == 0) return data.img.tile_rock13;
    if (n == 0 && s == 1 && w == 1 && e == 1) return data.img.tile_rock14;
    if (n == 0 && s == 0 && w == 0 && e == 0) return data.img.tile_rock15;
    if (n == 1 && s == 1 && w == 1 && e == 1) return data.img.tile_rock16;
    return data.img.tile_rock;
}







// *********************************************************************
// Button events
// *********************************************************************

function playButton () {
    gamePaused = false;
}


function pauseButton () {
    gamePaused = true;
}


function startButton () {
    actRound = 0;
    gamePreview = true;
}


function endButton () {
    actRound = 99999;
    gamePreview = true;
}


function animButton () {
    gameAnim = !gameAnim;
}

function closeButton () {
    window.close();
}





// *********************************************************************
// Mouse events
// *********************************************************************

function onDocumentMouseWheel (event) {
    var speed = 1;
    // WebKit
    if ( event.wheelDeltaY ) {
        actRound -= event.wheelDeltaY * speed;
	// Opera / Explorer 9
    } else if ( event.wheelDelta ) {
        actRound -= event.wheelDelta * speed;
	// Firefox
    } else if ( event.detail ) {
        actRound -= event.detail * speed;
    }
    mainloop();
}



// *********************************************************************
// Key events
// *********************************************************************


function onDocumentKeyDown (event) {
}




function onDocumentKeyUp (event) {

    // http://www.webonweboff.com/tips/js/event_key_codes.aspx

    switch (event.keyCode) {

    case 36: // Start
        actRound = 0;
        gamePreview = true;
        break;

    case 35: // End
        actRound = 99999;
        gamePreview = true;
        break;

    case 33: // PageDown
        actRound -= 20;
        gamePreview = true;
        break;

    case 34: // PageUp
        actRound += 20;
        gamePreview = true;
        break;

    case 38: // ArrowUp
    case 37: // ArrowLeft
        gamePaused= true;
        frame = 0;
        --actRound;
        gamePreview = true;
        break;

    case 40: // ArrowDown
    case 39: // ArrowRight
        gamePaused = true;
        frame = 0;
        ++actRound;
        gamePreview = true;
        break;

    case 32: // Space
        gamePaused = !gamePaused;
        mainloop();
        break;

    case 27: // Escape
        gamePaused = true;
        mainloop();
        break;

    case 187: // "+"
        reduction_factor += 0.15;
        break;

    case 189: // "-"
        reduction_factor -= 0.15;
        break;

    case 72: // "h"
        help();
        break;

    default:
        $("#debug").html(event.keyCode);
        break;
    }
}




function onWindowResize (event) {
    drawGame();
}


function help () {
    // opens a new popup with the help page
    var win = window.open('help.html' , 'name', 'height=400, width=300');
    if (window.focus) win.focus();
    return false;
}



// *********************************************************************
// This function is called periodically.
// *********************************************************************

function mainloop () {

    // Configure buttons
    if (gamePaused) {
        $("#but_play").show();
        $("#but_pause").hide();
    } else {
        $("#but_play").hide();
        $("#but_pause").show();
    }

    if (dataLoaded) {
        $("#but_anim").show();
        $("#but_reverse").show();
    } else {
        $("#but_anim").hide();
        $("#but_reverse").hide();
    }

    // initialitzation
    if (dataLoaded && !gameLoaded) initGame();
    if (!gameLoaded) return;

    // check actRound.
    if (actRound < 0) actRound = 0;
    if (actRound >= data.nb_rounds - 1) actRound = data.nb_rounds - 1;


    if (!gamePaused || gamePreview) {
        if (gamePreview) {
            frames = 0;
            gamePreview = false;
        }
        frames += gameDirection;
        if (frames == FRAMES_PER_TURN || frames == 0) {
            if (actRound < data.nb_rounds) actRound += gameDirection;
            frames = 0;
        }
        if (actRound < 0) actRound = 0;
        if (actRound >= data.nb_rounds - 1) actRound = data.nb_rounds - 1;

        updateGame();
        drawGame();
    }

    // write score board
    var score = "<table style='table-layout:fixed; background-color:#ffffff; width: 600; ' cellpadding=2>";
    var size = 30;
    score += "<col width='" + size + "'>";     // Goku
    score += "<col width='10'>";               // Blank
    score += "<col width='100'>";              // Name
    score += "<col width='10'>";               // Blank
    score += "<col width='75'>";               // Score
    score += "<col width='20'>";               // Blank
    score += "<col width='"+ size + "'>";      // Ball
    score += "<col width='5'>";                // Blank
    score += "<col width='25'>";               // Nb. balls
    score += "<col width='20'>";               // Blank
    score += "<col width='100'>";              // Strength
    score += "<col width='20'>";               // Blank
    score += "<col width='27'>";               // Bomb
    score += "<col width='110'>";              // CPU

    for (var i = 0; i < data.nb_players; ++i) {

        score += "<tr style='line-height: " + size + "px;'>";

        score += "<td style='background-color:#ffffff;'>";
        if (data.rounds[actRound].team[i].state == 'a') {
            score += "<img width='" + size + "px' src='img/goku-"+colors[i]+".png'/>";
        } else {
            sz = Math.min(size, size - size * data.rounds[actRound].team[i].time / data.goku_regen_time);
            ro = 180 * data.rounds[actRound].team[i].time / data.goku_regen_time;
            if (i % 2) ro = -ro;
            score += "<center><img width='"+sz+"px' style='-webkit-transform: rotate("+ro+"deg);' src='img/goku-"+colors[i]+".png'/></center>";
        }
        score += "</td>";

        score += "<td style='background-color:#ffffff;'></td>";

        score += "<td style='background-color:#ffffff; color: "+colors[i]+"'><b>"+ data.names[i] + "</b></td>";

        score += "<td style='background-color:#ffffff;'></td>";

        score += "<td style='background-color:#ffffff; text-align: right; color: "+colors[i]+"'><b>"+ (data.rounds[actRound].team[i].balls * (1 + data.max_strength) + data.rounds[actRound].team[i].strength) + "</b></td>";

        score += "<td style='background-color:#ffffff;'></td>";

        score += "<td style='background-color:#ffffff;'>";
        score += "<img width='" + Math.round(0.8*size) +"px' src='img/ball.png'/>";
        score += "</td>";

        score += "<td style='background-color:#ffffff;'></td>";

        score += "<td style='background-color:#ffffff; color: "+colors[i]+"'><b>"+ data.rounds[actRound].team[i].balls + "</b></td>";

        score += "<td style='background-color:#ffffff;'></td>";

        score += "<td style='background-color:#ffffff;'>";
        score += "<span style='display: inline-block; background-color:" + colors[i] +"; width: " +((100.0 * data.rounds[actRound].team[i].strength) / data.max_strength) + "; height: 10px;'/>";
        score += "</td>";

        score += "<td style='background-color:#ffffff;'></td>";

        score += "<td style='background-color:#ffffff;'>";
        if (data.rounds[actRound].cpu[i] < 0) {
            c = 0;
            score += "<img width='25px' src='img/bomb.png'/>";
        }
        score += "</td>";

        var c = int(data.rounds[actRound].cpu[i]*100);
        if (c > 100 || c < 0) c = 100;
        score += "<td style='background-color:#ffffff;'>";
        score += "<span style='display: inline-block; background-color: yellow;   width: " +(c) + "; height: 25px;'/>";
        score += "</td>";

        score += "</tr>";
    }
    score += "</table>";
    document.getElementById("scoreBoard").innerHTML = score;

    // write round
    $("#round").html(actRound);
}





// *********************************************************************
// This function is called when the DOM is ready.
// *********************************************************************

function init () {

    $("#debug").hide();


    // round things off
    $("#loading").show();
    $("#canvas").hide();
    $("#scoreboard").hide();

    // prepare the slider
    $("#slider").slider({
        slide: function(event, ui) {
            var value = $("#slider").slider( "option", "value" );
            actRound = value;
        }
    });
    $("#slider").width(500);

    // get url parameters
    var game;
    if (getURLParameter("sub") != "null") {
        if (getURLParameter("nbr") != "null") {
            game = "https://boladedrac-fib.jutge.org/?cmd=lliuraments&sub="+getURLParameter("sub")+"&nbr="+getURLParameter("nbr")+"&download=partida";
        } else {
            game = "https://boladedrac-fib.jutge.org/?cmd=partida&sub="+getURLParameter("sub")+"&download=partida";
        }
    } else {
        game = getURLParameter("game");
    }

    var fac = getURLParameter("fac");
    if (fac != "null") reduction_factor = double(fac);

    // load the given game
    loadGame(game);

    // round all on
    $("#all").show();

    // set the listerners for interaction
    document.addEventListener('mousewheel', onDocumentMouseWheel, false);
    document.addEventListener('keydown', onDocumentKeyDown, false);
    document.addEventListener('keyup', onDocumentKeyUp, false);

    window.addEventListener('resize', onWindowResize, false);

    // periodically call mainloop
    setInterval(mainloop, FRAME_TIME);

};
