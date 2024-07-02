#version 410 core
#ifdef GL_ES
precision mediump float;
#endif

uniform float time;

// camera position
uniform float camPosX;
uniform float camPosY;
uniform float camPosZ;

// elgonate vec3
uniform float elongateX;
uniform float elongateY;
uniform float elongateZ;

// pyramid size
uniform float pyramidSize;
uniform float basePyramidSize;
#define basePyramidSize basePyramidSize

// sphere position
uniform float spherePosX;
uniform float spherePosY;
uniform float spherePosZ;

// sphere size
uniform float sphereSize;
#define sphereSize sphereSize

// camera rotation
uniform float cameraX;
uniform float cameraY;

uniform float pyramid_fractal_offset_x;
uniform float pyramid_fractal_offset_y;
uniform float pyramid_fractal_offset_z;

#define camPos vec3(camPosX * 5.0 - 2.5, camPosY * 5.0 - 2.5, camPosZ * 5.0 - 2.5)
#define cameraRotation vec2(cameraX * 3.14159265358979323846 * 10.0, cameraY * 3.14159265358979323846 * 10.0)
#define fractal_x_offset pyramid_fractal_offset_x * 2.0 - 1.0
#define fractal_y_offset pyramid_fractal_offset_y * 2.0 - 1.0
#define fractal_z_offset pyramid_fractal_offset_z * 2.0 - 1.0
#define pyramidPos vec3(fractal_x_offset, fractal_y_offset, fractal_z_offset)
#define spherePos vec3(spherePosX * 5.0 - 2.5, spherePosY * 5.0 - 2.5, spherePosZ * 5.0 - 2.5)

in vec2 uv;

out vec4 fragColor;

// ok

mat3 calculateEyeRayTransformationMatrix(in vec3 ro, in vec3 ta, in float roll) {
    vec3 ww = normalize(ta - ro);
    vec3 uu = normalize(cross(ww, vec3(sin(roll), cos(roll), 0.0)));
    vec3 vv = normalize(cross(uu, ww));
    return mat3(uu, vv, ww);
}

vec2 sdfBalloon(vec3 currentRayPosition) {

    vec3 balloonPosition = vec3(0., 0., -.4);

    float balloonRadius = .9;

    float distanceToBalloon = length(currentRayPosition - balloonPosition);

    float distanceToBalloonSurface = distanceToBalloon - balloonRadius;

    float balloonID = 1.;

    vec2 balloon = vec2(distanceToBalloonSurface, balloonID);

    return balloon;

}

vec2 sdfBox(vec3 currentRayPosition) {

    vec3 boxPosition = vec3(-.8, -.4, 0.2);

    vec3 boxSize = vec3(.4, .3, .2);

    vec3 adjustedRayPosition = currentRayPosition - boxPosition;

    vec3 distanceVec = abs(adjustedRayPosition) - boxSize;
    float maxDistance = max(distanceVec.x, max(distanceVec.y, distanceVec.z));
    float distanceToBoxSurface = min(maxDistance, 0.0) + length(max(distanceVec, 0.0));

    float boxID = 2.;

    vec2 box = vec2(distanceToBoxSurface, boxID);

    return box;

}

vec2 whichThingAmICloserTo(vec2 thing1, vec2 thing2) {

    vec2 closestThing;

    if(thing1.x <= thing2.x) {

        closestThing = thing1;

    } else if(thing2.x < thing1.x) {

        closestThing = thing2;

    }

    return closestThing;

}

vec2 mapTheWorld(vec3 currentRayPosition) {

    vec2 result;

    vec2 balloon = sdfBalloon(currentRayPosition);
    vec2 box = sdfBox(currentRayPosition);

    result = whichThingAmICloserTo(balloon, box);

    return result;

}

const float HOW_CLOSE_IS_CLOSE_ENOUGH = 0.001;

const float FURTHEST_OUR_RAY_CAN_REACH = 40.;

const int HOW_MANY_STEPS_CAN_OUR_RAY_TAKE = 100;

vec2 checkRayHit(in vec3 eyePosition, in vec3 rayDirection) {

    float distanceToSurface = HOW_CLOSE_IS_CLOSE_ENOUGH * 2.;

    float totalDistanceTraveledByRay = 0.;

    float finalDistanceTraveledByRay = -1.;

    float finalID = -1.;

    for(int i = 0; i < HOW_MANY_STEPS_CAN_OUR_RAY_TAKE; i++) {

        if(distanceToSurface < HOW_CLOSE_IS_CLOSE_ENOUGH)
            break;

        if(totalDistanceTraveledByRay > FURTHEST_OUR_RAY_CAN_REACH)
            break;

        vec3 currentPositionOfRay = eyePosition + rayDirection * totalDistanceTraveledByRay;

        vec2 distanceAndIDOfThingsInTheWorld = mapTheWorld(currentPositionOfRay);

        float distanceToThingsInTheWorld = distanceAndIDOfThingsInTheWorld.x;
        float idOfClosestThingInTheWorld = distanceAndIDOfThingsInTheWorld.y;

        distanceToSurface = distanceToThingsInTheWorld;

        finalID = idOfClosestThingInTheWorld;

        totalDistanceTraveledByRay += distanceToThingsInTheWorld;

    }

    if(totalDistanceTraveledByRay < FURTHEST_OUR_RAY_CAN_REACH) {
        finalDistanceTraveledByRay = totalDistanceTraveledByRay;
    }

    if(totalDistanceTraveledByRay > FURTHEST_OUR_RAY_CAN_REACH) {
        finalDistanceTraveledByRay = FURTHEST_OUR_RAY_CAN_REACH;
        finalID = -1.;
    }

    return vec2(finalDistanceTraveledByRay, finalID);

}

vec3 getNormalOfSurface(in vec3 positionOfHit) {

    vec3 tinyChangeX = vec3(0.001, 0.0, 0.0);
    vec3 tinyChangeY = vec3(0.0, 0.001, 0.0);
    vec3 tinyChangeZ = vec3(0.0, 0.0, 0.001);

    float upTinyChangeInX = mapTheWorld(positionOfHit + tinyChangeX).x;
    float downTinyChangeInX = mapTheWorld(positionOfHit - tinyChangeX).x;

    float tinyChangeInX = upTinyChangeInX - downTinyChangeInX;

    float upTinyChangeInY = mapTheWorld(positionOfHit + tinyChangeY).x;
    float downTinyChangeInY = mapTheWorld(positionOfHit - tinyChangeY).x;

    float tinyChangeInY = upTinyChangeInY - downTinyChangeInY;

    float upTinyChangeInZ = mapTheWorld(positionOfHit + tinyChangeZ).x;
    float downTinyChangeInZ = mapTheWorld(positionOfHit - tinyChangeZ).x;

    float tinyChangeInZ = upTinyChangeInZ - downTinyChangeInZ;

    vec3 normal = vec3(tinyChangeInX, tinyChangeInY, tinyChangeInZ);

    return normalize(normal);
}

vec3 doBackgroundColor() {
    return vec3(0.);
}

vec3 doBalloonColor(vec3 positionOfHit, vec3 normalOfSurface) {

    vec3 sunPosition = vec3(1., 4., 3.);

    vec3 lightDirection = sunPosition - positionOfHit;

    lightDirection = normalize(lightDirection);

    float faceValue = dot(lightDirection, normalOfSurface);

    faceValue = max(0., faceValue);

    // vec3 balloonColor = vec3(1., 0., 0.);
    // make a shiny sun
    vec3 balloonColor = vec3(1., 1., 1.);

    // make it shiny
    faceValue = pow(faceValue, 5.);

    vec3 color = balloonColor * faceValue;

    color += vec3(.3, .1, .2);

    return color;
}

vec3 doBoxColor(vec3 positionOfHit, vec3 normalOfSurface) {

    vec3 color = vec3(normalOfSurface.x, normalOfSurface.y, normalOfSurface.z);

    return color;
}

vec3 colorTheWorld(vec2 rayHitInfo, vec3 eyePosition, vec3 rayDirection) {

    vec3 color;

    if(rayHitInfo.y < 0.0) {

        color = doBackgroundColor();

        // use distance to color background
        color = vec3(rayHitInfo.x / FURTHEST_OUR_RAY_CAN_REACH);

    } else {

        vec3 positionOfHit = eyePosition + rayHitInfo.x * rayDirection;

        vec3 normalOfSurface = getNormalOfSurface(positionOfHit);

        if(rayHitInfo.y == 1.0) {

            color = doBalloonColor(positionOfHit, normalOfSurface);

        } else if(rayHitInfo.y == 2.0) {

            color = doBoxColor(positionOfHit, normalOfSurface);

        }

    }

    return color;

}

void main() {

    // xy Position Of Pixel In Window
    vec2 p = uv * 2. - 1.;

    vec3 eyePosition = vec3(0., 0., 2.);

    vec3 pointWeAreLookingAt = vec3(0., 0., 0.);

    mat3 eyeTransformationMatrix = calculateEyeRayTransformationMatrix(eyePosition, pointWeAreLookingAt, 0.);

    vec3 rayComingOutOfEyeDirection = normalize(eyeTransformationMatrix * vec3(p.xy, 2.));

    vec2 rayHitInfo = checkRayHit(eyePosition, rayComingOutOfEyeDirection);

    vec3 color = colorTheWorld(rayHitInfo, eyePosition, rayComingOutOfEyeDirection);

    fragColor = vec4(color, 1.0);

}
