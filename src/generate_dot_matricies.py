from PIL import ImageFont, Image, ImageDraw

ahg = "AlteHaasGroteskRegular.ttf"
vcr = "VCR_OSD_MONO_1.001.ttf"
mono = "FragmentMono-Regular.ttf"
js = "JoganSoft-Regular.ttf"
dd = "Daydream.ttf"

def get_dot_matrix(characters, font_path=__name__.replace("generate_dot_matricies.py", './fonts/'+dd), font_size=32, spread=2):
    
    font = ImageFont.truetype(font_path, font_size)

    # Create a temporary image to measure text size
    temp_image = Image.new('L', (1, 1))
    draw = ImageDraw.Draw(temp_image)
    
    # Measure text dimensions using textbbox
    bbox = draw.textbbox((0, 0), characters, font=font)
    text_width = bbox[2] - bbox[0]
    text_height = bbox[3] - bbox[1]
    
    # Create an image with the exact size of the text
    image = Image.new('L', (text_width, int(text_height*1.3333)), 0)
    draw = ImageDraw.Draw(image)
    draw.text((0, 0), characters, font=font, fill=255)
    

    # Convert image to dot-matrix (binary)
    matrix = []
    pixels = image.load()
    for y in range(image.size[1]):
        row = []
        for x in range(image.size[0]):
            if pixels[x, y] > 16 and x % spread == 0 and y % spread == 0:  # Threshold for converting to binary
                row.append(1)
            else:
                row.append(0)
        matrix.append(row)
    
    return matrix

if __name__ == "__main__":
    strs_to_generate = []
    for hour in range(24):
        if hour < 10:
            hour = "0"+str(hour)
        else:
            hour = str(hour)
        for minu in range(60):
            if minu < 10:
                minu = "0"+str(minu)
            else:
                minu = str(minu)
            strs_to_generate.append(hour+":"+minu)

    char_to_matrix_map = {}

    for stri in strs_to_generate:
        char_to_matrix_map[stri] = get_dot_matrix(stri)

    with open(__name__.replace("generate_dot_matricies.py", "../generated_matricies/gen.txt"), 'w') as file:
        file.write(str(char_to_matrix_map))
    # with open("./generated_matricies/gen.txt", 'r') as file:
    #     tx = file.read()
    # print(ast.literal_eval(tx)['1'])