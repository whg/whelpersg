#include "ofxFlexibleVideoPlayer.h"

#include "ofxTools.h"


static const string shaderVersion = "#version 150\n";
static const string vertShader = GLSL150(
    uniform mat4 modelViewProjectionMatrix;

    in vec4  position;
    in vec2  texcoord;

    out vec2 texCoordVarying;

    void main() {
        texCoordVarying = (vec4(texcoord.x,texcoord.y,0,1)).xy;
        gl_Position = modelViewProjectionMatrix * position;
    }
);


static const string fragShader = GLSL150(
    uniform sampler2DRect texA;
    uniform sampler2DRect texB;
    uniform float blend;
    uniform vec2 size;

    in vec2 texCoordVarying;

    out vec4 fragColor;

    void main(){
        fragColor = mix(texture(texA, texCoordVarying * size), texture(texB, texCoordVarying * size), blend);
    }
);


ofxFlexibleSilentVideoPlayer::ofxFlexibleSilentVideoPlayer():
mLastUpdateTime(0),
mFrameRate(0),
mLoop(LoopType::END),
//mAudioStep(1),
mSpeed(1) {}

// framesFolder: a directory of images, in the right order, alphabetical?
// audioFile: a .wav file of the soundtrack
void ofxFlexibleSilentVideoPlayer::load(string framesFolder, float frameRate) {
    
    ofDirectory dir(framesFolder);
    ofImage img;
    for (auto &file : dir.getFiles()) {
        auto path = file.getAbsolutePath();
        ofTexture tex;
        img.load(path);
        tex.loadData(img.getPixels());
        mTextures.push_back(std::move(tex));
        
        cout << file.getAbsolutePath() << endl;
    }
    
    ofLogNotice("ofxFlexibleSilentVideoPlayer") << "loaded " << mTextures.size() << " images";
    

	
    mFrameRate = frameRate;
    mFrameTime = 1.0f / frameRate; // this should be from the frameRate of the video and not change
    mContentLength = int(mTextures.size()) * mFrameTime;
    
    /////////////////////////////////////

	
    blendShader.setupShaderFromSource(GL_VERTEX_SHADER, vertShader);
    blendShader.setupShaderFromSource(GL_FRAGMENT_SHADER, fragShader);
    blendShader.bindDefaults();
    blendShader.linkProgram();
    
    ofSetWindowShape(mTextures[0].getWidth(), mTextures[0].getHeight());
}

void ofxFlexibleVideoPlayer::load(string framesFolder, string audioFile, float frameRate) {

	ofxFlexibleSilentVideoPlayer::load(framesFolder, frameRate);
	
	//    mSoundtrackSample.load(audioFile);
	//
	//    mAudioData.resize(mSoundtrackSample.length);
	//    for (int i = 0; i < mAudioData.size(); i++) {
	//        mAudioData[i] = mSoundtrackSample.temp[i] / 32767.0f;
	//    }}
	
	//    mSoundtrackSample.getLength();
	//    auto audiolength = mSoundtrackSample.length;
	
}

void ofxFlexibleSilentVideoPlayer::update() {
    
    auto currentTime = ofGetElapsedTimef();
    auto timeSinceLastUpdate = currentTime - mLastUpdateTime;
    mLastUpdateTime = ofGetElapsedTimef();
    
    auto lastPlayhead = mPlayhead;
    mPlayhead+= timeSinceLastUpdate * mSpeed;
	
    // simple loopback
    if (mLoop == LoopType::WHOLE) {
        if (mPlayhead >= mContentLength) {
            setPositionTime(0);
        }
        else if (mPlayhead < 0) {
            setPositionTime(mContentLength);
        }
    }
    else if (mLoop == LoopType::END) {
        if (mPlayhead >= mContentLength) {
            setPositionTime(mCuePoint);
        }
    }
        
}

void ofxFlexibleVideoPlayer::update() {
	
	auto currentTime = ofGetElapsedTimef();
	auto timeSinceLastUpdate = currentTime - mLastUpdateTime;
	auto lastPlayhead = mPlayhead;

	ofxFlexibleSilentVideoPlayer::update();
	
	audioMutex.lock();
	auto playheadDiff = (mPlayhead - lastPlayhead);
	mAudioStep = playheadDiff / timeSinceLastUpdate;
	audioMutex.unlock();
}

void ofxFlexibleSilentVideoPlayer::draw() {
    
    float exactFrame = mPlayhead / mFrameTime;
    int frameA = int(floor(exactFrame));
    int frameB = int(ceil(exactFrame));
    
    frameA %= mTextures.size();
    frameB %= mTextures.size();
    
    if (frameB == frameA) {
        if (mSpeed > 0) {
            frameB = (frameA + 1 + mTextures.size()) % mTextures.size();
        }
        else {
            frameA = (frameA - 1 + mTextures.size()) % mTextures.size();
        }
    }
    
    assert(frameA != frameB);
    
    float blend = (exactFrame - frameA) / float(frameB - frameA);
        
    assert(frameA >= 0 && frameA < mTextures.size());
    
    ofVec2f texSize(mTextures[0].getWidth(), mTextures[0].getHeight());
    ofMesh mesh = ofMesh::plane(texSize.x, texSize.y);
    
    ofTranslate(texSize / 2);
    blendShader.begin();

    blendShader.setUniformTexture("texA", mTextures[frameA], 0);
    blendShader.setUniformTexture("texB", mTextures[frameB], 1);
    blendShader.setUniform1f("blend", blend);
    blendShader.setUniform2f("size", texSize);
    mesh.drawFaces();
    
    blendShader.end();

//    printf("%d, %d, %f\n", frameA, frameB, blend);
}

void ofxFlexibleVideoPlayer::audioOut(ofSoundBuffer& buffer) {
    
    ofScopedLock lock(audioMutex);
    
    auto &data = buffer.getBuffer();
    auto mainDataLength = mAudioData.size();

    int nChannels = buffer.getNumChannels();
    auto l = buffer.size() / nChannels;
    auto it = mAudioData.begin() + mAudioPlayhead;
    // TODO: fix for variable channels
    for (int i = 0; i < l; i++) {
        float pos = (mAudioPlayhead + mAudioStep * i);
        if (pos < 0) pos+= mainDataLength;
        float a = mAudioData[int(floor(pos)) % mainDataLength];
        float b = mAudioData[int(ceil(pos)) % mainDataLength];
        float blend = pos - floor(pos);
        data[i*2] = data[i*2+1] = a * (1.f - blend) + b * blend;
    }
    
    mAudioPlayhead+= l * mAudioStep;
    if (mAudioPlayhead >= mainDataLength) {
        mAudioPlayhead-= mainDataLength;
    }
    else if (mAudioPlayhead < mainDataLength) {
        mAudioPlayhead+= mainDataLength;
    }
}

void ofxFlexibleSilentVideoPlayer::setPositionTime(float time) {

    mPlayhead = time;
}

void ofxFlexibleVideoPlayer::setPositionTime(float time) {

	ofxFlexibleSilentVideoPlayer::setPositionTime(time);
	
	audioMutex.lock();
	mAudioPlayhead = (mPlayhead / mContentLength) * mAudioData.size();
	audioMutex.unlock();
}

void ofxFlexibleSilentVideoPlayer::setFrame(int frame) {
    setPositionTime(frame * mFrameTime);
}

void ofxFlexibleSilentVideoPlayer::setPosition(float point) {
    setPositionTime(point * mContentLength);
}


