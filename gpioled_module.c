#include <linux/fs.h>                              /* open( ), read( ), write( ), close( ) 커널 함수 */
#include <linux/cdev.h>                          /* 문자 디바이스 */
#include <linux/module.h>
#include <linux/io.h>                              /* ioremap( ), iounmap( ) 커널 함수 */
#include <asm/uaccess.h>                    /* copy_to_user( ), copy_from_user( ) 커널 함수 */


MODULE_LICENSE("GPL");
MODULE_AUTHOR("KKY");
MODULE_DESCRIPTION("ODROID C1 GPIO LED Device Module");

#define	GPIO_REG_MAP		0xC1108000

static volatile unsigned int	*gpio;
static char msg[BLOCK_SIZE] = {0};                      /* write( ) 함수에서 읽은 데이터 저장 */

#define GPIOY_FSEL_REG_OFFSET   0x0F
#define GPIOY_OUTP_REG_OFFSET   0x10
#define GPIOY_INP_REG_OFFSET    0x11
#define GPIOY_PUPD_REG_OFFSET   0x3D
#define GPIOY_PUEN_REG_OFFSET   0x4B

#define	MUX_REG1		0x2D
#define	MUX_REG2		0x2E
#define	MUX_REG3		0x2F
#define	MUX_REG4		0x30
#define	MUX_REG5		0x31
#define	MUX_REG6		0x32
#define	MUX_REG7		0x33
#define	MUX_REG8		0x34
#define	MUX_REG9		0x35

/* 장치 파일의 주번호와 부번호 */
#define GPIO_MAJOR 		201
#define GPIO_MINOR 		0
#define GPIO_DEVICE             "gpioled"              /* 디바이스 장치 파일의 이름 */
#define GPIO_LED                3                         /* LED 사용을 위한 GPIO의 번호 */

#define GPIO_SIZE                 (128)   //

/* 입출력 함수를 위한 선언 */
static int gpio_open(struct inode *, struct file *);
static ssize_t gpio_read(struct file *, char *, size_t, loff_t *);
static ssize_t gpio_write(struct file *, const char *, size_t, loff_t *);
static int gpio_close(struct inode *, struct file *);

/* 유닉스 입출력 함수들의 처리를 위한 구조체 */
static struct file_operations gpio_fops = {
   .owner = THIS_MODULE,
   .read = gpio_read,
   .write = gpio_write,
   .open = gpio_open,
   .release = gpio_close,
};

struct cdev gpio_cdev; 

static int gpio_open(struct inode *inod, struct file *fil)
{
    printk("GPIO Device opened(%d/%d)\n", imajor(inod), iminor(inod));

    return 0;
}

static int gpio_close(struct inode *inod, struct file *fil)
{
    printk("GPIO Device closed(%d)\n", MAJOR(fil->f_dentry->d_inode->i_rdev));

    return 0;
}

static ssize_t gpio_read(struct file *inode, char *buff, size_t len, loff_t *off)
{
    int count = 0;

    return count;
}

static ssize_t gpio_write(struct file *inode, const char *buff, size_t len, loff_t *off)
{
    short count = 0;
	memset(msg, 0, BLOCK_SIZE);
    count = copy_from_user(msg, buff, len);        /* 유저 영역으로 부터 데이터를 가져온다. */

    /* 사용자가 보낸 데이터가 0인 경우 LED를 끄고, 0이 아닌 경우 LED를 켠다. */
    (!strcmp(msg, "0"))? (*(gpio+GPIOY_OUTP_REG_OFFSET) &= ~(1 << GPIO_LED)) : (*(gpio+GPIOY_OUTP_REG_OFFSET) |=  (1 << GPIO_LED));
    return count;
}

int initModule(void)
{
    dev_t devno;
    unsigned int count;
    static void *map;                                   /* I/O 접근을 위한 변수 */
    int err;

    printk(KERN_INFO "Hello module!\n");

    try_module_get(THIS_MODULE);

    /* 문자 디바이스를 등록한다. */
    devno = MKDEV(GPIO_MAJOR, GPIO_MINOR);
    register_chrdev_region(devno, 1, GPIO_DEVICE);

    /* 문자 디바이스를 위한 구조체를 초기화한다. */
    cdev_init(&gpio_cdev, &gpio_fops);

    gpio_cdev.owner = THIS_MODULE;
    count = 1;
    err = cdev_add(&gpio_cdev, devno, count);               /* 문자 디바이스를 추가한다. */
    if (err < 0) {
        printk("Error : Device Add\n");
        return -1;
    }

    printk("'mknod /dev/%s c %d 0'\n", GPIO_DEVICE, GPIO_MAJOR);
    printk("'chmod 666 /dev/%s'\n", GPIO_DEVICE);

    map = ioremap(GPIO_REG_MAP, GPIO_SIZE);              /* 사용할 메모리를 할당한다. */
    if(!map) {
        printk("Error : mapping GPIO memory\n");
        iounmap(map);
        return -EBUSY;
    }

    gpio = (volatile unsigned int *)map;

	//초기화 코드 필요
    //GPIO_IN(GPIO_LED);                                               /* LED 사용을 위한 초기화 */
    //GPIO_OUT(GPIO_LED);

    return 0;
}

void cleanupModule(void)
{
    dev_t devno = MKDEV(GPIO_MAJOR, GPIO_MINOR);
    unregister_chrdev_region(devno, 1);       /* 문자 디바이스의 등록을 해제한다. */

    cdev_del(&gpio_cdev);                                /* 문자 디바이스의 구조체를 해제한다. */

    if (gpio) {
       iounmap(gpio);                                          /* 매핑된 메모리를 삭제한다. */
    }

    module_put(THIS_MODULE);

    printk(KERN_INFO "Good-bye module!\n");
}

module_init(initModule);
module_exit(cleanupModule);