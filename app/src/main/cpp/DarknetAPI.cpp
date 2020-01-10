#include <ctime>
#include <jni.h>
#include <android/log.h>
#include "DarknetAPI.h"
#include "RecogNumber.h"

#define RAD2DEG 57.32484
#define  LOG_TAG    "Darknet"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)

// Initialize the parameters
float confThreshold = 0.1; // Confidence threshold
float nmsThreshold = 0.4;  // Non-maximum suppression threshold
int inpWidth = 416;        // Width of network's input image
int inpHeight = 416;       // Height of network's input image
std::string detection_classes[2] = { "CAR","MOTORBIKE" };
std::string recognition_classes[36] = { "0", "1", "2", "3", "4", "5","6", "7","8", "9","A", "B","C", "D","E", "F","G",
								"H","I", "J","K", "L","M", "N","O", "P","Q", "R","S", "T","U", "V","W", "X","Y", "Z" };


DarknetAPI::DarknetAPI(char *detect_cfgfile, char *detect_weightfile,
	char *carlp_cfgfile, char *carlp_weightfile,
	char *motorlp_cfgfile, char *motorlp_weightfile)
{
	// Load the network
	detection_net = cv::dnn::readNetFromDarknet(std::string(detect_cfgfile), std::string(detect_weightfile));
	detection_net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
	detection_net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);

	car_lp_net = parse_network_cfg_custom(carlp_cfgfile, 1); // set batch=1
	if (carlp_weightfile) {
		load_weights(&car_lp_net, carlp_weightfile);
	}

	motor_lp_net = parse_network_cfg_custom(motorlp_cfgfile, 1); // set batch=1
	if (motorlp_weightfile) {
		load_weights(&motor_lp_net, motorlp_weightfile);
	}
	srand(2222222);
}

void DarknetAPI::sort_by_value(float *value, int size, int increase)
{
	int i, j;
	float temp;
	for (i = 0; i < size; i++)
	{
		for (j = i; j < size; j++)
		{
			if (increase)
			{
				if (value[i] > value[j])
				{
					temp = value[i];
					value[i] = value[j];
					value[j] = temp;
				}
			}
			else
			{
				if (value[i] < value[j])
				{
					temp = value[i];
					value[i] = value[j];
					value[j] = temp;
				}
			}
		}
	}
}
std::vector<CharacterGroup> DarknetAPI::RemovOverlappedCharacter(image im, detection_with_class * selected_detections, int * det_num)
{
    int num = *det_num;
    int i;

    std::vector<CharacterGroup> result;
    result.clear();
    if (num < 1) return result;
    CharacterGroup temp;
    temp.best_class = selected_detections[0].best_class;
    temp.label = recognition_classes[selected_detections[0].best_class];
    temp.pos = selected_detections[0].det.bbox;
    temp.prob = selected_detections[0].det.prob[selected_detections[0].best_class];
    result.push_back(temp);

    for (i = 1; i < num; i++)
    {
        box b0 = selected_detections[i - 1].det.bbox;
        int left0 = (b0.x - b0.w / 2.)*im.w;
        int right0 = (b0.x + b0.w / 2.)*im.w;
        int top0 = (b0.y - b0.h / 2.)*im.h;
        int bot0 = (b0.y + b0.h / 2.)*im.h;

        if (left0 < 0) left0 = 0;
        if (right0 > im.w - 1) right0 = im.w - 1;
        if (top0 < 0) top0 = 0;
        if (bot0 > im.h - 1) bot0 = im.h - 1;

        box b1 = selected_detections[i].det.bbox;
        int left1 = (b1.x - b1.w / 2.)*im.w;
        int right1 = (b1.x + b1.w / 2.)*im.w;
        int top1 = (b1.y - b1.h / 2.)*im.h;
        int bot1 = (b1.y + b1.h / 2.)*im.h;

        if (left1 < 0) left1 = 1;
        if (right1 > im.w - 1) right1 = im.w - 1;
        if (top1 < 0) top1 = 1;
        if (bot1 > im.h - 1) bot1 = im.h - 1;

        if (abs(left0 - left1) < 1 || abs(right0 - right1) < 1)
        {
            continue;
        }

        temp.best_class = selected_detections[i].best_class;
        temp.label = recognition_classes[selected_detections[i].best_class];
        temp.pos = selected_detections[i].det.bbox;
        temp.prob = selected_detections[i].det.prob[selected_detections[i].best_class];
        result.push_back(temp);
    }

    return result;
}

std::vector<CharacterGroup> DarknetAPI::RemovOverlappedCharacterForMotor(image im, std::vector<CharacterGroup> result)
{
	int i;
	std::vector<CharacterGroup> removed_result;
	removed_result.clear();

	CharacterGroup temp;
	temp.best_class = result[0].best_class;
	temp.label = result[0].label;
	temp.pos = result[0].pos;
	temp.prob = result[0].prob;
	removed_result.push_back(temp);

	for (i = 1; i < result.size(); i++)
	{
		box b0 = result[i - 1].pos;
		int centerx0 = b0.x*im.w;
		int centery0 = b0.y*im.h;
		int width0 = b0.w * im.w;

		box b1 = result[i].pos;
		int centerx1 = b1.x*im.w;
		int centery1 = b1.y*im.h;
		int width1 = b1.w * im.w;

		float dist = sqrt(pow(centerx0 - centerx1, 2) + pow(centery1 - centery0, 2));
		if (dist < (width0 * 0.25f + width1 * 0.25f))
		{
			continue;
		}

		temp.best_class = result[i].best_class;
		temp.label =result[i].label;
		temp.pos = result[i].pos;
		temp.prob = result[i].prob;
		removed_result.push_back(temp);
	}

	return removed_result;
}

std::vector<CharacterGroup> DarknetAPI::detections_for_characters_of_car(image im, detection *dets, int num, float thresh, std::string *names, int classes, int ext_output)
{
	int selected_detections_num;
	std::vector<CharacterGroup> result;
	result.clear();

	detection_with_class* selected_detections = get_actual_detections(dets, num, thresh, &selected_detections_num);
	// text output
	qsort(selected_detections, selected_detections_num, sizeof(*selected_detections), compare_by_lefts);
	std::vector<CharacterGroup> trueCharacters;
	trueCharacters = RemovOverlappedCharacter(im, selected_detections, &selected_detections_num);

	int i, j;
	int best_class;
	float probabilites[20] = { 0.0f, };
	if (trueCharacters.size() > 10)  // if detect more than 10 characters
	{
		for (i = 0; i < trueCharacters.size(); i++)
		{
			best_class = trueCharacters[i].best_class;
			probabilites[i] = trueCharacters[i].prob;
		}
		//sort probabilties for each class of detections
		sort_by_value(probabilites, trueCharacters.size(), 0);
	}
	for (i = 0; i < trueCharacters.size(); ++i) {
		CharacterGroup tmp_strt;
		//string label = character_names[selected_detections[i].best_class];
		tmp_strt.label = recognition_classes[trueCharacters[i].best_class];
		if (trueCharacters[i].prob < probabilites[9]) continue;
		tmp_strt.pos = trueCharacters[i].pos;
		tmp_strt.prob = trueCharacters[i].prob;
		result.push_back(tmp_strt);
	}
	free(selected_detections);
	return result;
}


std::vector<CharacterGroup> DarknetAPI::detections_for_characters_of_motorbike(image im, detection *dets, int num, float thresh, std::string *names, int classes, int ext_output)
{
	int selected_detections_num;
	std::vector<CharacterGroup> result;
	result.clear();

	detection_with_class* selected_detections = get_actual_detections(dets, num, thresh, &selected_detections_num);
	// text output
	qsort(selected_detections, selected_detections_num, sizeof(*selected_detections), compare_by_lefts);
	std::vector<CharacterGroup> trueCharacters;
	int i, j;
	for (i = 0; i < selected_detections_num; i++)
	{
		CharacterGroup temp;
		temp.best_class = selected_detections[i].best_class;
		temp.label = recognition_classes[selected_detections[i].best_class];
		temp.pos = selected_detections[i].det.bbox;
		temp.prob = selected_detections[i].det.prob[selected_detections[i].best_class];
		trueCharacters.push_back(temp);
	}
	
	int best_class;
	float probabilites[20] = { 0.0f, };
	if (trueCharacters.size() > 10)  // if detect more than 10 characters
	{
		for (i = 0; i < trueCharacters.size(); i++)
		{
			best_class = trueCharacters[i].best_class;
			probabilites[i] = trueCharacters[i].prob;
		}
		//sort probabilties for each class of detections
		sort_by_value(probabilites, trueCharacters.size(), 0);
	}
	for (i = 0; i < trueCharacters.size(); ++i) {
		CharacterGroup tmp_strt;
		//string label = character_names[selected_detections[i].best_class];
		tmp_strt.label = recognition_classes[trueCharacters[i].best_class];
		if (trueCharacters[i].prob < probabilites[8]) continue;
		tmp_strt.pos = trueCharacters[i].pos;
		tmp_strt.prob = trueCharacters[i].prob;

		box b = trueCharacters[i].pos;
		int center_y = b.y * im.h;
		if (center_y < im.h / 2) result.push_back(tmp_strt);
	}

	for (i = 0; i < trueCharacters.size(); ++i) {
		CharacterGroup tmp_strt;
		//string label = character_names[selected_detections[i].best_class];
		tmp_strt.label = recognition_classes[trueCharacters[i].best_class];
		if (trueCharacters[i].prob < probabilites[8]) continue;
		tmp_strt.pos = trueCharacters[i].pos;
		tmp_strt.prob = trueCharacters[i].prob;

		box b = trueCharacters[i].pos;
		int center_y = b.y * im.h;
		if (center_y > im.h / 2) result.push_back(tmp_strt);
	}
	
	std::vector<CharacterGroup> removed_result = RemovOverlappedCharacterForMotor(im, result);
	free(selected_detections);
	return removed_result;
}

std::vector<CharacterGroup> DarknetAPI::characters_detector_of_car(Mat plate_mat, float thresh, int ext_output)
{
	std::vector<CharacterGroup> result;
	srand(2222222);
	double time;
	char buff[256];
	char *input = buff;
	int j;
	float nms = .45;    // 0.4F

						//image im = load_image(filename, 0, 0, plate_net.c);
	int letterbox = 0;
	image sized = resize_image(mat_to_image(plate_mat), car_lp_net.w, car_lp_net.h);
	layer l = car_lp_net.layers[car_lp_net.n - 1];

	float *X = sized.data;

	//time= what_time_is_it_now();
	//double time = get_time_point();
	network_predict(car_lp_net, X);
	//printf("%s: Predicted in %lf milli-seconds.\n", input, ((double)get_time_point() - time) / 1000);
	//printf("%s: Predicted in %f seconds.\n", input, (what_time_is_it_now()-time));

	int nboxes = 0;
	detection *dets = get_network_boxes(&car_lp_net, sized.w, sized.h, thresh, 0, 0, 1, &nboxes, letterbox);
	if (nms) do_nms_sort(dets, nboxes, l.classes, nms);
	result = detections_for_characters_of_car(sized, dets, nboxes, thresh, recognition_classes, l.classes, ext_output);


	free_detections(dets, nboxes);
	free_image(sized);
	return result;

}

std::vector<CharacterGroup> DarknetAPI::characters_detector_of_motorbike(Mat plate_mat, float thresh, int ext_output)
{
	std::vector<CharacterGroup> result;
	srand(2222222);
	double time;
	char buff[256];
	char *input = buff;
	int j;
	float nms = .45;    // 0.4F

						//image im = load_image(filename, 0, 0, plate_net.c);
	int letterbox = 0;
	image sized = resize_image(mat_to_image(plate_mat), motor_lp_net.w, motor_lp_net.h);
	layer l = motor_lp_net.layers[motor_lp_net.n - 1];

	float *X = sized.data;

	//time= what_time_is_it_now();
	//double time = get_time_point();
	network_predict(motor_lp_net, X);
	//printf("%s: Predicted in %lf milli-seconds.\n", input, ((double)get_time_point() - time) / 1000);
	//printf("%s: Predicted in %f seconds.\n", input, (what_time_is_it_now()-time));

	int nboxes = 0;
	detection *dets = get_network_boxes(&motor_lp_net, sized.w, sized.h, thresh, 0, 0, 1, &nboxes, letterbox);
	if (nms) do_nms_sort(dets, nboxes, l.classes, nms);
	result = detections_for_characters_of_motorbike(sized, dets, nboxes, thresh, recognition_classes, l.classes, ext_output);


	free_detections(dets, nboxes);
	free_image(sized);
	return result;

}

std::vector<Result> DarknetAPI::LicensePlateRecognition(Mat lp_mat, float lp_thresh, int lp_type)
{
	std::vector<Result> results;

	network * lp_net = NULL;
	if (lp_type == 1)
	{
		lp_net = &car_lp_net;
	}
	else if (lp_type == 2)
	{
		lp_net = &motor_lp_net;
	}

	image lp_image = mat_to_image(lp_mat);

	srand(2222222);
	float nms = .45;    // 0.4F
	int letterbox = 0;
	image sized = resize_image(lp_image, lp_net->w, lp_net->h);
	layer l = lp_net->layers[lp_net->n - 1];

	float *X = sized.data;
	/*double time = get_time_point();*/
	network_predict(*lp_net, X);

	int nboxes = 0;
	detection *dets = get_network_boxes(lp_net, sized.w, sized.h, lp_thresh, 0, 0, 1, &nboxes, letterbox);
	if (nms) do_nms_sort(dets, nboxes, l.classes, nms);
	std::vector<CharacterGroup> characterGroup;
	if (lp_type == 1) //long
		characterGroup = detections_for_characters_of_car(sized, dets, nboxes, lp_thresh, recognition_classes, l.classes, 1);
	else if (lp_type == 2) //short
		characterGroup = detections_for_characters_of_motorbike(sized, dets, nboxes, lp_thresh, recognition_classes, l.classes, 1);

	int i;
	int left, right, top, bot;
	Result temp;
	for (i = 0; i < characterGroup.size(); ++i) {
		temp.label = characterGroup[i].label;
		box b = characterGroup[i].pos;
		left = (b.x - b.w / 2.)*sized.w;
		right = (b.x + b.w / 2.)*sized.w;
		top = (b.y - b.h / 2.)*sized.h;
		bot = (b.y + b.h / 2.)*sized.h;
		if (left < 0) left = 1;
		if (right > sized.w - 1) right = sized.w - 1;
		if (top < 0) top = 1;
		if (bot > sized.h - 1) bot = sized.h - 1;
		temp.left = left;
		temp.right = right;
		temp.top = top;
		temp.bot = bot;
		temp.confidence = characterGroup[i].prob;
		results.push_back(temp);
	}
	free_detections(dets, nboxes);
	free_image(lp_image);
	free_image(sized);
	return results;
}

int DarknetAPI::getRotAngle(Mat input, Mat &output)
{
	//********************************************************
	Mat cropedMat;
	input.copyTo(cropedMat);

	//detected image affine
	//convert image to gray
	Point a(0, 0);   // corner point A
	Point b(cropedMat.cols, cropedMat.rows); // corner point B
	float angle = 0.f;

	RotatedRect rrect(0.5*(a + b), // center
		Size2f((float)fabs(a.x - b.x), fabs(a.y - b.y)), // size
		angle);
	Mat result_mat = cropedMat.clone();
	circle(result_mat, rrect.center, 3, Scalar(0, 255, 0), -1);
	//get the min size between width and height
	float minSize = (rrect.size.width < rrect.size.height) ? rrect.size.width : rrect.size.height;
	minSize = minSize - minSize * 0.45;
	float maxSize = (rrect.size.width < rrect.size.height) ? rrect.size.height : rrect.size.width;
	maxSize = maxSize - maxSize * 0.45;
	//Initialize floodfill parameters and variables
	Mat mask;
	mask.create(cropedMat.rows + 2, cropedMat.cols + 2, CV_8UC1);
	mask = Scalar::all(0);
	int loDiff = 50;
	int upDiff = 50;
	int connectivity = 8;
	int newMaskVal = 255;
	int NumSeeds = 10;	// @f 10 -> 50
	Rect ccomp;
	int flags = connectivity + (newMaskVal << 8) + CV_FLOODFILL_FIXED_RANGE + CV_FLOODFILL_MASK_ONLY;

	for (int j = -NumSeeds / 2; j < NumSeeds / 2; j++) {
		Point seed;
		seed.x = rrect.center.x + (int)maxSize / (NumSeeds)*j;
		//seed.y = rrect.center.y + (int)minSize / (NumSeeds)*j;
		seed.y = rrect.center.y;

		circle(result_mat, seed, 5, Scalar(0, 255, 255), -1);	// @f 1 -> 5
		int area = floodFill(cropedMat, mask, seed, Scalar(255, 0, 0), &ccomp, Scalar(loDiff, loDiff, loDiff), Scalar(upDiff, upDiff, upDiff), flags);

	}
	// 		imshow("result_mat", result_mat);
	// 		cvWaitKey(1);
			//Check new floodfill mask match for a correct patch.
			//Get all points detected for get Minimal rotated Rect
	std::vector<Point> pointsInterest;
	Mat_<uchar>::iterator itMask = mask.begin<uchar>();
	Mat_<uchar>::iterator end = mask.end<uchar>();
	for (; itMask != end; ++itMask)
		if (*itMask == 255)
			pointsInterest.push_back(itMask.pos());
	RotatedRect minRect = minAreaRect(pointsInterest);

	// rotated rectangle drawing
	Point2f rect_points[4]; minRect.points(rect_points);
	for (int j = 0; j < 4; j++)
		line(result_mat, rect_points[j], rect_points[(j + 1) % 4], Scalar(0, 0, 255), 1, 8);
	//Get rotation matrix

	float r = (float)minRect.size.width / (float)minRect.size.height;
	angle = minRect.angle;
	if (angle < -45) {
		angle += 90.0;
		swap(minRect.size.width, minRect.size.height);
	}
	Mat rotmat = getRotationMatrix2D(minRect.center, angle, 1);
//	imshow("rot_mat", result_mat);
//	waitKey(1);
	//Create and rotate image

	Mat img_rotated;
	warpAffine(cropedMat, img_rotated, rotmat, cropedMat.size(), INTER_CUBIC);
	// 		imshow("img_rotated", img_rotated);
	// 		cvWaitKey(1);
			//Crop image
	Size rect_size = minRect.size;
	Mat img_crop, ori_crop;
	Size ori_size = rect_size;
	//rect_size.width = rect_size.width*1.15;
	/*rect_size.height = rect_size.height*1.15;
	getRectSubPix(img_rotated, rect_size, minRect.center, img_crop);*/
	getRectSubPix(img_rotated, ori_size, minRect.center, ori_crop);
	//Equalize croped image
	Mat rcroped_img;
	if (ori_crop.cols < 10 || ori_crop.rows < 10) {
		rcroped_img = cropedMat.clone();
	}
	else {
		rcroped_img = ori_crop.clone();

	}
	LOGI("JNI: rotate image with width  %d   and height   %d", rcroped_img.cols, rcroped_img.rows);
	rcroped_img.copyTo(output);
	return (int)angle;
}


// Get the names of the output layers
std::vector<String> DarknetAPI::getOutputsNames(const cv::dnn::Net& net)
{
	std::vector<String> names;
	if (names.empty())
	{
		//Get the indices of the output layers, i.e. the layers with unconnected outputs
		std::vector<int> outLayers = net.getUnconnectedOutLayers();

		//get the names of all the layers in the network
		std::vector<String> layersNames = net.getLayerNames();

		// Get the names of the output layers in names
		names.resize(outLayers.size());
		for (size_t i = 0; i < outLayers.size(); ++i)
			names[i] = layersNames[outLayers[i] - 1];
	}
	return names;
}

std::vector<Result> DarknetAPI::LicensePlateDetect(Mat input) {
	LOGI("JNI: detect image with width  %d   and height   %d", input.cols, input.rows);
	Mat blob;

	// Create a 4D blob from a frame.
	cv::dnn::blobFromImage(input, blob, 1 / 255.0, Size(inpWidth, inpHeight), Scalar(0, 0, 0), true, false);

	//Sets the input to the network
	detection_net.setInput(blob);

	// Runs the forward pass to get output of the output layers
	std::vector<Mat> outs;
	detection_net.forward(outs, getOutputsNames(detection_net));

	std::vector<Result> results;

	std::vector<int> classIds;
	std::vector<float> confidences;
	std::vector<Rect> boxes;

	for (size_t i = 0; i < outs.size(); ++i)
	{
		// Scan through all the bounding boxes output from the network and keep only the
		// ones with high confidence scores. Assign the box's class label as the class
		// with the highest score for the box.
		float* data = (float*)outs[i].data;
		for (int j = 0; j < outs[i].rows; ++j, data += outs[i].cols)
		{
			Mat scores = outs[i].row(j).colRange(5, outs[i].cols);
			Point classIdPoint;
			double confidence;
			// Get the value and location of the maximum score
			minMaxLoc(scores, 0, &confidence, 0, &classIdPoint);
			if (confidence > confThreshold)
			{
				int centerX = (int)(data[0] * input.cols);
				int centerY = (int)(data[1] * input.rows);
				int width = (int)(data[2] * input.cols);
				int height = (int)(data[3] * input.rows);
				int left = centerX - width / 2;
				int top = centerY - height / 2;

				classIds.push_back(classIdPoint.x);
				confidences.push_back((float)confidence);
				boxes.push_back(Rect(left - (int)(width * 0.05f), top - (int)(height * 0.05f), (int)(width * 1.1f), (int)(height * 1.1f)));
			}
		}
	}

	// Perform non maximum suppression to eliminate redundant overlapping boxes with
	// lower confidences
	std::vector<int> indices;
	cv::dnn::NMSBoxes(boxes, confidences, confThreshold, nmsThreshold, indices);

	//string img_save_dir_1 = "D:/My2019Work/VariousTrain/VietnamCarPlate/PlateRecognition/Long/JPEGImages/";
	//string img_save_dir_2 = "D:/My2019Work/VariousTrain/VietnamCarPlate/PlateRecognition/Short/JPEGImages/";

	if (indices.size() < 1) return results;

	float max_confidence = confidences[indices[0]];
	int max_id = 0;
	for (size_t i = 0; i < indices.size(); ++i)
	{
		int idx = indices[i];
		if (max_confidence < confidences[idx])
		{
			max_confidence = confidences[idx];
			max_id = idx;
		}
	}

	Rect bbox = boxes[max_id];

	int left, top, right, bottom;
	left = bbox.x;
	if (left < 1) left = 1;
	top = bbox.y;
	if (top < 1) top = 1;
	right = left + bbox.width;
	bottom = top + bbox.height;

	if (right > input.cols) right = input.cols - 1;
	if (bottom > input.rows) bottom = input.rows - 1;

	Result temp;
	temp.left = left;
	temp.right = right;
	temp.top = top;
	temp.bot = bottom;
	temp.confidence = confidences[max_id];
	temp.label = detection_classes[classIds[max_id]];
	results.push_back(temp);

	return results;
}

DarknetAPI::~DarknetAPI()
{
}
