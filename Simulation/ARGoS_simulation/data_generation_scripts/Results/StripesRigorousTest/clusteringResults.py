from PIL import Image
from pdf2image import convert_from_path

proportions = ["ni35_b15_w0","ni35_b11_w4","ni34_b8_w8"]
samplingTime = ["2s","10s"]
secondParam = ["125","175","225"]
imgs =[]
imgsWidth = []
imgsHeight = []

for i in samplingTime:
    for j in secondParam:
        tempImg = convert_from_path('05_' + j + '_' + i + '_' + proportions[0] +'/Convergence.pdf')
        tempWidth, tempHeight = tempImg[0].size
        imgsWidth.append(tempWidth)
        imgsHeight.append(tempHeight)
        imgs.append(tempImg[0])

w = max(imgsWidth)
h = max(imgsHeight)

# create big empty image with place for images
new_image = Image.new('RGB', (w*3, h*2))

# put images on new_image
new_image.paste(imgs[0], (0, 0))
new_image.paste(imgs[1], (w, 0))
new_image.paste(imgs[2], (w*2, 0))
new_image.paste(imgs[3], (0, h))
new_image.paste(imgs[4], (w, h))
new_image.paste(imgs[5], (w*2, h))

# save it
new_image.save(proportions[0] + '_convergence.pdf')
