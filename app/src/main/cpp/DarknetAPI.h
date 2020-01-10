#pragma once

#include <string>
#include <vector>

extern "C" {
#include "network.h"
#include "parser.h"
#include "utils.h"
}

#include "image_opencv.h"
#include "http_stream.h"

struct Result {
	int left;
	int top;
	int right;
	int bot;
	float confidence;
	std::string label;
};

typedef struct _tagCharacterGroup
{
	std::string label;
	int		best_class;
	box		pos;
	float   prob;
}CharacterGroup;

class DarknetAPI
{

private:
	network car_lp_net;
	network motor_lp_net;
	cv::dnn::Net detection_net;

public:
	DarknetAPI(char *detect_cfgfile, char *detect_weightfile,
		char *carlp_cfgfile, char *carlp_weightfile,
		char *motorlp_cfgfile, char *motrlp_weightfile);
	~DarknetAPI();


	std::vector<Result> LicensePlateDetect(Mat input);
	std::vector<Result> LicensePlateRecognition(Mat lp_mat, float lp_thresh,
		int lp_type);
	void sort_by_value(float *value, int size, int increase);
	std::vector<CharacterGroup> RemovOverlappedCharacter(image im, detection_with_class * selected_detections, int * det_num);
	std::vector<CharacterGroup> RemovOverlappedCharacterForMotor(image im, std::vector<CharacterGroup> result);
	int getRotAngle(Mat input, Mat &output);
	std::vector<String> getOutputsNames(const cv::dnn::Net& net);

	std::vector<CharacterGroup> detections_for_characters_of_car(image im, detection *dets, int num, float thresh, std::string *names, int classes, int ext_output);
	std::vector<CharacterGroup> detections_for_characters_of_motorbike(image im, detection *dets, int num, float thresh, std::string *names, int classes, int ext_output);
	std::vector<CharacterGroup> characters_detector_of_car(Mat plate_mat, float thresh, int ext_output);
	std::vector<CharacterGroup> characters_detector_of_motorbike(Mat plate_mat, float thresh, int ext_output);
};

