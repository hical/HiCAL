function AwayTimer(showAlert){
    if(showAlert === undefined)
        showAlert = false;

    this.awayTime = 0;
    this.backTime = 0;
    this.hiddenTime = 0;
    this.totalAwayTime = 0;
    this.startTime = Date.now();

    var parent = this;

    parent.resetTimer= function(){
        parent.startTime = Date.now();
        parent.awayTime = 0;
        parent.totalAwayTime = 0;
    };

    var awayCallback = function() {
        parent.awayTime = Date.now();
        if(showAlert){
            showAlarm("Are you still here? " +
                "If your are still available, please click ok to continue.");
        }
    };

    var awayBackCallback = function() {
        parent.backTime = Date.now();
        totalTime =0;
        if(parent.awayTime != 0){
            totalTime = parent.backTime-parent.awayTime;
        }
        parent.totalAwayTime = parent.totalAwayTime+ totalTime;
        parent.awayTime =  0;
        parent.backTime = 0;
    };

    var hiddenCallback = function() {
        parent.hiddenTime = Date.now();
    };

    var visibleCallback = function(){
        parent.backTime = Date.now();
        totalTime = 0;
        if(parent.awayTime != 0 && hiddenTime != 0){
            if(parent.awayTime > hiddenTime){
                totalTime = parent.backTime-parent.hiddenTime;
            }else{
                totalTime = parent.backTime-parent.awayTime;
            }
        }else if(parent.hiddenTime != 0){
            totalTime = parent.backTime-parent.hiddenTime;
        }else if(parent.awayTime != 0){
            totalTime = parent.backTime-parent.awayTime;
        }


        parent.totalAwayTime+=totalTime;
        parent.awayTime =  0;
        parent.backTime = 0;
        parent.hiddenTime = 0;
    };

    parent.idle = new Idle({
        onHidden : hiddenCallback,
        onVisible : visibleCallback,
        onAway : awayCallback,
        onAwayBack : awayBackCallback,
        awayTimeout : 60000 //away with default value of the textbox
    }).start();
    return parent;
}
