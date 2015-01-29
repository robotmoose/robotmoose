var processingInstance;
var piezo, therm, light, rear_range, ir_therm1, ir_therm2, bump_wheel, cd1, cd2, cd3, cd4, cd5;

function addValue() {
    if (!processingInstance) {
        processingInstance = Processing.getInstanceById("processing1");
    }
    processingInstance.pushData();
}

var getTimeString = function() {
    return new Date();
};