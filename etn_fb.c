#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/fb.h>
#include <linux/io.h>
#include <linux/init.h>
#include <linux/backlight.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>
#include <linux/dma-mapping.h>

#include <asm/page.h>

#include "etn_fb.h"

#define DEVNAME                     "test_int"

static int fps = 25;
static int use_fpga2sdram = 1;

module_param(fps, int, S_IRUGO);
module_param(use_fpga2sdram, int, S_IRUGO);

static u32 etn_fb_pseudo_palette[16];
u32 *fpga_regs;


#define ADRR_REG_VIDEO_MEM_ADDR    0
#define ADRR_REG_VIDEO_MEM_SIZE    1
#define ADRR_REG_REFRESH_RATE      2
#define ADRR_REG_CTRL              3
#define ADRR_REG_CMD               4
#define ADRR_REG_DATA              5

#define CTRL_REG_RESET             (1<<0)
#define CTRL_REG_LCD_ON            (1<<1)
#define CTRL_REG_REFRESH_ON        (1<<2)

//#define LCD_WRITE(addr,value) iowrite32(value, fpga_regs+ addr)
//fpga_write_reg(addr,value)

#define  LCD_WR_REG(value)   LCD_WRITE(ADRR_REG_CMD,value)
#define  LCD_WR_DATA(value)  LCD_WRITE(ADRR_REG_DATA,value)

#define  Set_LCD_RST  LCD_WRITE(ADRR_REG_CTRL,CTRL_REG_RESET | CTRL_REG_LCD_ON)
#define  Clr_LCD_RST  LCD_WRITE(ADRR_REG_CTRL,CTRL_REG_LCD_ON)

#define LCD_SET_ADDR(addr)  LCD_WRITE(ADRR_REG_VIDEO_MEM_ADDR,addr)
#define LCD_SET_SIZE(size)  LCD_WRITE(ADRR_REG_VIDEO_MEM_SIZE,size)
#define LCD_SET_REFRESH_RATE(refresh_rate)  LCD_WRITE(ADRR_REG_REFRESH_RATE,refresh_rate)

#define DISPLAY_WIDTH                     (320)
#define DISPLAY_HEIGHT                    (240)


static void LCD_WRITE(int reg, u32 val)
{
	iowrite32(val, fpga_regs+ reg);
}

#if 0
static u16 fpga_read_reg(int reg)
{
	u16 tmp;

	tmp = ioread16(fpga_regs);// + 2*reg);
	return tmp;
}

static void fpga_set_bit(int reg, int bit)
{
	unsigned long tmp = fpga_read_reg(reg);

	set_bit(bit, &tmp);
	fpga_write_reg(reg, tmp);
}

static void fpga_clear_bit(int reg, int bit)
{
	unsigned long tmp = fpga_read_reg(reg);

	clear_bit(bit, &tmp);
	fpga_write_reg(reg, tmp);
}

static void lcd_write_command(u16 val)
{
	/* Write command code */
	fpga_write_reg(LCD_DATA_CR, val);

	/* WR and RS low, RD high */
	fpga_write_reg(LCD_CTRL_CR, LCD_CTRL_CR_RD);
	ndelay(1);

	/* RS low, WR and RD high */
	fpga_write_reg(LCD_CTRL_CR, LCD_CTRL_CR_RD | LCD_CTRL_CR_WR);
	ndelay(1);

	/* All control signals high */
	fpga_write_reg(LCD_CTRL_CR, LCD_CTRL_CR_RD | LCD_CTRL_CR_WR |
                       LCD_CTRL_CR_RS);
}

static void lcd_write_data(u16 data)
{
	/* Write data */
	fpga_write_reg(LCD_DATA_CR, data);

	/* WR low, RD and RS high */
	fpga_write_reg(LCD_CTRL_CR, LCD_CTRL_CR_RD | LCD_CTRL_CR_RS);
	ndelay(1);

	/* All control signals high */
	fpga_write_reg(LCD_CTRL_CR, LCD_CTRL_CR_RD |
		       LCD_CTRL_CR_RS | LCD_CTRL_CR_WR);
}
/* See ILI9341 Datasheet */
static void lcd_init(struct fb_info *info)
{
	/* Clear data */
	fpga_write_reg(LCD_DATA_CR, 0);

	/* All control signals high */
	fpga_write_reg(LCD_CTRL_CR, LCD_CTRL_CR_RD |
		       LCD_CTRL_CR_RS | LCD_CTRL_CR_WR);

	mdelay(100);

	lcd_write_command(ILI9341_DISPLAY_ON);

	lcd_write_command(ILI9341_SLEEP_OUT);
	lcd_write_command(ILI9341_MEM_ACCESS_CTRL);
	lcd_write_data(MY | MX | MV | BGR);

	lcd_write_command(ILI9341_PIXEL_FORMAT);
	lcd_write_data(0x0055);

	lcd_write_command(ILI9341_COLUMN_ADDR);
	lcd_write_data(0x0000);
	lcd_write_data(0x0000);
	lcd_write_data((DISPLAY_WIDTH-1) >> 8);
	lcd_write_data((DISPLAY_WIDTH-1) & 0xFF);

	lcd_write_command(ILI9341_PAGE_ADDR);
	lcd_write_data(0x0000);
	lcd_write_data(0x0000);
	lcd_write_data((DISPLAY_HEIGHT-1) >> 8);
	lcd_write_data((DISPLAY_HEIGHT-1) & 0xFF);

	lcd_write_command(ILI9341_MEM_WRITE);
}
#endif
void LCD_Init(void)
{
//	alt_u16 data1,data2;
//	alt_u16 data3,data4;
	Clr_LCD_RST;
	mdelay(1);
	Set_LCD_RST;
	mdelay(10);       // Delay 10ms // This delay time is necessary
	Clr_LCD_RST;
	mdelay(120);       // Delay 120 ms
//	Clr_LCD_CS;



	LCD_WR_REG(0x0011); //Exit Sleep
	LCD_WR_REG(0x00CF);
		LCD_WR_DATA(0x0000);
		LCD_WR_DATA(0x0081);
		LCD_WR_DATA(0X00c0);

	LCD_WR_REG(0x00ED);
		LCD_WR_DATA(0x0064);
		LCD_WR_DATA(0x0003);
		LCD_WR_DATA(0X0012);
		LCD_WR_DATA(0X0081);

	LCD_WR_REG(0x00E8);
		LCD_WR_DATA(0x0085);
		LCD_WR_DATA(0x0001);
		LCD_WR_DATA(0x00798);

	LCD_WR_REG(0x00CB);
		LCD_WR_DATA(0x0039);
		LCD_WR_DATA(0x002C);
		LCD_WR_DATA(0x0000);
		LCD_WR_DATA(0x0034);
		LCD_WR_DATA(0x0002);

	LCD_WR_REG(0x00F7);
		LCD_WR_DATA(0x0020);

	LCD_WR_REG(0x00EA);
		LCD_WR_DATA(0x0000);
		LCD_WR_DATA(0x0000);

	LCD_WR_REG(0x00B1);
		LCD_WR_DATA(0x0000);
		LCD_WR_DATA(0x001b);

	LCD_WR_REG(0x00B6);
		LCD_WR_DATA(0x000A);
		LCD_WR_DATA(0x00A2);

	LCD_WR_REG(0x00C0);    //Power control
		LCD_WR_DATA(0x0005);   //VRH[5:0]

	LCD_WR_REG(0x00C1);    //Power control
		LCD_WR_DATA(0x0011);   //SAP[2:0];BT[3:0]

	LCD_WR_REG(0x00C5);    //VCM control
		LCD_WR_DATA(0x0045);       //3F
		LCD_WR_DATA(0x0045);       //3C

	 LCD_WR_REG(0x00C7);    //VCM control2
		 LCD_WR_DATA(0X00a2);

	LCD_WR_REG(0x0036);    // Memory Access Control
		LCD_WR_DATA(0x0068);//48
//  	        LCD_WR_DATA(MY | MX | MV | BGR);

	LCD_WR_REG(0x00F2);    // 3Gamma Function Disable
		LCD_WR_DATA(0x0000);

	LCD_WR_REG(0x0026);    //Gamma curve selected
		LCD_WR_DATA(0x0001);

	LCD_WR_REG(0x00E0);    //Set Gamma
		LCD_WR_DATA(0x000F);
		LCD_WR_DATA(0x0026);
		LCD_WR_DATA(0x0024);
		LCD_WR_DATA(0x000b);
		LCD_WR_DATA(0x000E);
		LCD_WR_DATA(0x0008);
		LCD_WR_DATA(0x004b);
		LCD_WR_DATA(0X00a8);
		LCD_WR_DATA(0x003b);
		LCD_WR_DATA(0x000a);
		LCD_WR_DATA(0x0014);
		LCD_WR_DATA(0x0006);
		LCD_WR_DATA(0x0010);
		LCD_WR_DATA(0x0009);
		LCD_WR_DATA(0x0000);

	LCD_WR_REG(0X00E1);    //Set Gamma
		LCD_WR_DATA(0x0000);
		LCD_WR_DATA(0x001c);
		LCD_WR_DATA(0x0020);
		LCD_WR_DATA(0x0004);
		LCD_WR_DATA(0x0010);
		LCD_WR_DATA(0x0008);
		LCD_WR_DATA(0x0034);
		LCD_WR_DATA(0x0047);
		LCD_WR_DATA(0x0044);
		LCD_WR_DATA(0x0005);
		LCD_WR_DATA(0x000b);
		LCD_WR_DATA(0x0009);
		LCD_WR_DATA(0x002f);
		LCD_WR_DATA(0x0036);
		LCD_WR_DATA(0x000f);

	LCD_WR_REG(0x002B);
		LCD_WR_DATA(0x0000);
		LCD_WR_DATA(0x0000);
		LCD_WR_DATA(0x0000);
		LCD_WR_DATA(0x00ef);

	 LCD_WR_REG(0x002A);
		LCD_WR_DATA(0x0000);
		LCD_WR_DATA(0x0000);
		LCD_WR_DATA(0x0001);
		LCD_WR_DATA(0x003f);

	LCD_WR_REG(0x003A);
		LCD_WR_DATA(0x0055);

	LCD_WR_REG(0x00f6);
		LCD_WR_DATA(0x0001);
		LCD_WR_DATA(0x0030);
		LCD_WR_DATA(0x0000);

	LCD_WR_REG(0x0029); //display on

	LCD_WR_REG(0x002c);    // 0x2C

}


#define CNVT_TOHW(val, width) ((((val)<<(width))+0x7FFF-(val))>>16)

static int etn_fb_setcolreg(unsigned regno,
			    unsigned red, unsigned green, unsigned blue,
			    unsigned transp, struct fb_info *info)
{
	int ret = 1;

	if (info->var.grayscale)
		red = green = blue = (19595 * red + 38470 * green +
				      7471 * blue) >> 16;

	switch (info->fix.visual) {
	case FB_VISUAL_TRUECOLOR:
		if (regno < 16) {
			u32 *pal = info->pseudo_palette;
			u32 value;

			red = CNVT_TOHW(red, info->var.red.length);
			green = CNVT_TOHW(green, info->var.green.length);
			blue = CNVT_TOHW(blue, info->var.blue.length);
			transp = CNVT_TOHW(transp, info->var.transp.length);

			value = (red << info->var.red.offset) |
				(green << info->var.green.offset) |
				(blue << info->var.blue.offset) |
				(transp << info->var.transp.offset);

			pal[regno] = value;
			ret = 0;
		}
		break;
	case FB_VISUAL_STATIC_PSEUDOCOLOR:
	case FB_VISUAL_PSEUDOCOLOR:
		break;
	}

	return ret;
}

static struct fb_fix_screeninfo etn_fb_fix = {
	.id             = NAME,
	.type           = FB_TYPE_PACKED_PIXELS,
	.visual         = FB_VISUAL_TRUECOLOR,
	.accel          = FB_ACCEL_NONE,
	.line_length    = DISPLAY_WIDTH * DISPLAY_BPP / 8,
};

static struct fb_var_screeninfo etn_fb_var = {
	.width          = DISPLAY_WIDTH,
	.height         = DISPLAY_HEIGHT,
	.bits_per_pixel = DISPLAY_BPP,
	.xres           = DISPLAY_WIDTH,
	.yres           = DISPLAY_HEIGHT,
	.xres_virtual   = DISPLAY_WIDTH,
	.yres_virtual   = DISPLAY_HEIGHT,
	.activate       = FB_ACTIVATE_FORCE,
	.vmode          = FB_VMODE_NONINTERLACED,
	.red            = {11, 5, 0},
	.green          = {5, 6, 0},
	.blue           = {0, 5, 0},
};

int etn_fb_mmap(struct fb_info *info, struct vm_area_struct *vma)
{
	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

	return vm_iomap_memory(vma, info->fix.smem_start, info->fix.smem_len);
}

static struct fb_ops etn_fb_ops = {
	.owner          = THIS_MODULE,
	.fb_read        = fb_sys_read,
	.fb_write       = fb_sys_write,
	.fb_fillrect    = sys_fillrect,
	.fb_copyarea    = sys_copyarea,
	.fb_imageblit   = sys_imageblit,
	.fb_setcolreg   = etn_fb_setcolreg,
	.fb_mmap        = etn_fb_mmap,
};

static u64 platform_dma_mask = DMA_BIT_MASK(32);
#if 0
static void set_fps(int fps)
{
	unsigned int fps_delay;

	if (fps < MIN_FPS)
		fps = MIN_FPS;
	if (fps > MAX_FPS)
		fps = MAX_FPS;

	/* We set delay to FPGA in ms */
	fps_delay = 1000 / fps;

	fpga_write_reg(LCD_FPS_DELAY_CR, fps_delay);
}

static void set_dma_addr(dma_addr_t dma_addr)
{
	/* fpga2sdram interface has word address,
	 * but fpga2hps has byte address. */
	if (use_fpga2sdram)
		dma_addr = dma_addr / 8;

	/* Write address into FPGA-DMA */
	fpga_write_reg(LCD_DMA_ADDR_CR0, dma_addr & 0xFFFF);
	fpga_write_reg(LCD_DMA_ADDR_CR1, dma_addr >> 16);
}
#endif


void LCD_SetCursor(u16 Xpos, u16 Ypos)
{
	 LCD_WR_REG(0x002A);
		 LCD_WR_DATA(Xpos>>8);
		 LCD_WR_DATA(Xpos&0XFF);
	 LCD_WR_REG(0x002B);
		 LCD_WR_DATA(Ypos>>8);
		 LCD_WR_DATA(Ypos&0XFF);
	 LCD_WR_REG(0x002C);
}

void LCD_Clear(u16 Color)
{
        int index=0;
        LCD_SetCursor(0x00,0x0000);
        LCD_WR_REG(0x002C);
        for(index=0;index<76800;index++)
        {
        	LCD_WR_DATA(Color);
        }
}

static int etn_fb_probe(struct platform_device *pdev)
{
	struct fb_info *info;
	int ret;

	u32 vmem_size;
	unsigned char *vmem;

	dma_addr_t dma_addr;

      	printk(KERN_ALERT DEVNAME":video driver version %s- %s\n", __DATE__, __TIME__);
	pdev->dev.dma_mask = &platform_dma_mask;
	pdev->dev.coherent_dma_mask = DMA_BIT_MASK(32);

	vmem_size = (etn_fb_var.width * etn_fb_var.height * etn_fb_var.bits_per_pixel) / 8;

	vmem = dmam_alloc_coherent(&pdev->dev, vmem_size, &dma_addr, GFP_KERNEL);
	if (!vmem) {
		dev_err(&pdev->dev, "FB: dma_alloc_coherent error\n");
		return -ENOMEM;
	}

	memset(vmem, 0, vmem_size);

	info = framebuffer_alloc(0, &pdev->dev);
	if (!info)
		return -ENOMEM;

	info->screen_base = vmem;
	info->fbops = &etn_fb_ops;
	info->fix = etn_fb_fix;
	info->fix.smem_start = dma_addr;
	info->fix.smem_len = vmem_size;
	info->var = etn_fb_var;
	info->flags = FBINFO_DEFAULT;
	info->pseudo_palette = &etn_fb_pseudo_palette;

	/* Get FPGA registers address */
	fpga_regs = devm_ioremap(&pdev->dev, FPGA_REGS_BASE, REGSIZE);

	/* Disable refreshing */
	//fpga_write_reg(LCD_DMA_CR, 0);
        LCD_Init();

   LCD_Clear(0XF800);

//   return -1;

   LCD_SET_ADDR(dma_addr);
   LCD_SET_SIZE((38400-1));
   LCD_SET_REFRESH_RATE(50000000L);

   LCD_SetCursor(0x00,0x0000);
   LCD_WR_REG(0x002C);

   LCD_WRITE(ADRR_REG_CTRL,CTRL_REG_REFRESH_ON|CTRL_REG_LCD_ON);

   //	lcd_init(info);

//	set_dma_addr(dma_addr);

//	set_fps(fps);

	/* Enable refreshing */
//	fpga_set_bit(LCD_DMA_CR, LCD_DMA_CR_REDRAW_EN);

	ret = register_framebuffer(info);
	if (ret < 0) {
		framebuffer_release(info);
		return ret;
	}

	platform_set_drvdata(pdev, info);
      	printk(KERN_ERR DEVNAME":video driver end of probe\n");
	return 0;
}

static int etn_fb_remove(struct platform_device *pdev)
{
	struct fb_info *info = platform_get_drvdata(pdev);

	if (info) {
		unregister_framebuffer(info);

		framebuffer_release(info);
	}

	return 0;
}

static const struct of_device_id etn_of_match[] = {
	{ .compatible = "mtk,etn", },
	{.compatible="altr ,socfpga-mysoftip"},
	{},
};

MODULE_DEVICE_TABLE(of, etn_of_match);

static struct platform_driver etn_fb_driver = {
	.remove = etn_fb_remove,
	.driver = {
		.name   = NAME,
		.owner  = THIS_MODULE,
		.of_match_table = of_match_ptr(etn_of_match),
	},
};

static int __init etn_fb_init(void)
{
	if (platform_driver_probe(&etn_fb_driver, etn_fb_probe)) {
		printk(KERN_ERR "Failed to probe ETN platform driver\n");
		return -ENXIO;
	}
	return 0;
}

static void __exit etn_fb_exit(void)
{
	platform_driver_unregister(&etn_fb_driver);
}

MODULE_AUTHOR("Jury Remizov <jremizov@metrotek.spb.ru>");
MODULE_AUTHOR("Denis Gabidullin <d.gabidullin@metrotek.spb.ru>");
MODULE_DESCRIPTION("ETN LCD framebuffer driver");
MODULE_LICENSE("GPL");

module_init(etn_fb_init);
module_exit(etn_fb_exit);

