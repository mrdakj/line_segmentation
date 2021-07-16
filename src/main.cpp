#include <iostream>
#include "line_segmentation.h"

void process_image(const fs::path& img_name)
{
    std::cout << img_name << std::endl;
    try {
        image img(img_name);
        strips s(img);
        s.connect();
        auto strips_image = s.concatenate_strips();
        auto strips_contour = s.concatenate_strips_with_lines(false);
        auto strips_contour_original = s.concatenate_strips_with_lines(true);
        auto result_image = s.result();

        // img.crop();
        // img.show();
        // cv::waitKey(0);
        // img.save("1.png");
        // strips_image.show();
        // cv::waitKey(0);
        // strips_image.save("2.png");
        // strips_contour.show();
        // cv::waitKey(0);
        // strips_contour.save("3.png");
        // strips_contour_original.show();
        // cv::waitKey(0);
        // strips_contour_original.save("4.png");
        // result_image.show();
        // cv::waitKey(0);

        result_image.save(std::string("./out/") + std::string(img_name.filename()));
    } 
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
    }
}

int main(int argc, char** argv) 
{
    if (argc != 2) {
        std::cout << "usage: ./main path_to_image_or_dir" << std::endl;
        return -1;
    }

    if (!fs::exists("out")) {
        fs::create_directory("out");
    }

    fs::path p(argv[1]);
    if (fs::is_directory(p)) {
        for (auto it = fs::directory_iterator(p); it != fs::directory_iterator(); ++it) {
            process_image(*it);
        }
    }
    else {
        process_image(p);
    }

    return 0;
}
