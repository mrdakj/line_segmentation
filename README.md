# Line segmentation
Line segmentation algorithm.

![Alt text](out/5.png?raw=true "Example 1")

![Alt text](out/7.png?raw=true "Example 2")

![Alt text](out/25.png?raw=true "Example 3")

![Alt text](out/42.png?raw=true "Example 4")

***
## :package: Installation
:exclamation: Requirements: OpenCV

### Manual

1. Clone this repository somewhere on your machine.

    ```sh
    git clone https://github.com/mrdakj/line_segmentation.git

    ```
2. Compile

    ```sh
    cd line_segmentation
    mkdir build
    cd build
    cmake ..
    cmake --build .

    ```

3. Start

    ```sh
	./main path_to_image_or_dir

    ```

4. Example

    ```sh
	./main ../images
	./main ../images/7.png

    ```

***

## About the algorithm
This is implementation of the algorithm represented in the paper "Piece-wise Painting Technique for Line Segmentation of Unconstrained Handwritten Text: A Specific Study with Persian Text Documents" with modifications.
