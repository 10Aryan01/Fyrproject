const { PDFDocument, rgb } = require("pdf-lib");
const moment = require("moment");
const AWS = require("aws-sdk");
const fs = require("fs");

const S3_REGION = "eu-north-1";
const S3_BUCKET_NAME = "vehicleschallan";
const TWILIO_PHONE_NUMBER = "+16782646879";

const s3 = new AWS.S3({ region: S3_REGION });

const accountSid = "ACb61f358c4f26ca90a55137ada3807b2e";
const authToken = "153af9ed94e6ddc0f3248de888c8cc51";
const client = require("twilio")(accountSid, authToken);

async function generateBill(
  location,
  carNumber,
  ownerName,
  violatorName,
  speedData,
  challanAmount,
  minorDetails,
  phone_number,
  email,
  city,
  state,
  Challan_no,
  Payment_Status,
  Vehicle_name,
  Vehicle_type
) {
  const pdfDoc = await PDFDocument.create();
  const page = pdfDoc.addPage([600, 800]);
  const { width, height } = page.getSize();

  const titleSize = 16;
  const textSize = 12;
  let lineHeight = 20;

  let y = height - 40;

  const drawTitle = (title) => {
    page.drawText(title, {
      x: 50,
      y,
      size: titleSize,
      color: rgb(0, 0, 0),
    });
    y -= lineHeight;
  };

  const drawText = (text, x = 50) => {
    page.drawText(text, {
      x: 50,
      y,
      size: textSize,
      color: rgb(0, 0, 0),
    });
    y -= lineHeight;
  };

  const drawLine = () => {
    page.drawLine({
      start: { x: 50, y },
      end: { x: width - 50, y },
      thickness: 1,
      color: rgb(0, 0, 0),
    });
    y -= lineHeight + 10;
  };

  const drawHeading = (text) => {
    page.drawText(text, {
      x: 170,
      y,
      size: 30,
      color: rgb(0, 0, 0),
    });
    y -= lineHeight;
  };
  const subheading = (text, val) => {
    if (!val) {
      page.drawText(text, {
        x: 250,
        y,
        size: 12,
        color: rgb(0, 0, 0),
      });
    } else {
      page.drawText(text, {
        x: val,
        y,
        size: 12,
        color: rgb(0, 0, 0),
      });
    }
    y -= lineHeight;
  };

  page.drawLine({
    start: { x: 10, y: 10 },
    end: { x: width - 10, y: 10 },
    thickness: 2,
    color: rgb(0, 0, 0),
  });
  page.drawLine({
    start: { x: 10, y: 10 },
    end: { x: 10, y: height - 10 },
    thickness: 2,
    color: rgb(0, 0, 0),
  });
  page.drawLine({
    start: { x: width - 10, y: 10 },
    end: { x: width - 10, y: height - 10 },
    thickness: 2,
    color: rgb(0, 0, 0),
  });
  page.drawLine({
    start: { x: 10, y: height - 10 },
    end: { x: width - 10, y: height - 10 },
    thickness: 2,
    color: rgb(0, 0, 0),
  });

  drawHeading("Your challan details");
  lineHeight += 45;
  subheading(`Goverment of India`);
  lineHeight -= 45;

  drawTitle("Personal Details");
  drawLine();
  drawText(`Owner Name: ${ownerName}`);
  drawText(`Violator Name: ${violatorName}`);
  drawText(`Phone Number: ${phone_number}`);
  lineHeight += 35;
  drawText(`email: ${email}`);

  lineHeight -= 35;
  drawTitle("Penalty Information");
  drawLine();
  drawText(`Location: ${location}`);
  drawText(`Offences: ${minorDetails}`);
  const currentDate = moment().format("MMMM DD, YYYY");
  drawText(`Offence Date: ${currentDate}`);
  drawText(`Challan No: ${Challan_no}`);
  drawText(`Speed Data: ${speedData}`);
  drawText(`Compounding fees: ${challanAmount}`);
  lineHeight += 40;
  drawText(`Payment Status: ${Payment_Status}`);
  lineHeight -= 40;

  drawTitle("Car Information");
  drawLine();
  drawText(`Vehicle license No: ${carNumber}`);
  drawText(`Vehicle model name: ${Vehicle_name}`);
  drawText(`Vehicle type: ${Vehicle_type}`);
  drawText(`City: ${city}`);
  drawText(`State: ${state}`);
  lineHeight += 40;
  drawText("Country: India");
  lineHeight -= 40;
  drawLine();
  lineHeight -= 5;
  subheading("For any quries contact us on 84-54-999-999", 190);
  subheading("autochallan.traffic@assampolice.gov.in", 200);
  drawLine();

  const pdfBytes = await pdfDoc.save();

  const key = `bill_${moment().format("YYYYMMDDHHmmss")}.pdf`;

  await s3
    .upload({
      Bucket: S3_BUCKET_NAME,
      Key: key,
      Body: pdfBytes,
      ContentType: "application/pdf",
      ContentDisposition: 'attachment; filename="bill.pdf"',
    })
    .promise();

  const url = `https://${S3_BUCKET_NAME}.s3.${S3_REGION}.amazonaws.com/${key}`;

  return url;
}

async function sendSMS(billUrl, phoneNumber) {
  return new Promise((resolve, reject) => {
    client.messages
      .create({
        body: `Your bill is ready. You can download it from: ${billUrl}`,
        from: "+16782646879",
        to: phoneNumber,
      })
      .then((message) => {
        console.log("Message sent successfully. SID:", message.sid);
        resolve();
      })
      .catch((error) => {
        console.error("Error sending message:", error);
        reject(error);
      });
  });
}

exports.handler = async (event, context) => {
  try {
    const {
      location,
      carNumber,
      ownerName,
      violatorName,
      speedData,
      challanAmount,
      minorDetails,
      phone_number,
      email,
      city,
      state,
      Challan_no,
      Payment_Status,
      Vehicle_name,
      Vehicle_type,
    } = event;

    const billUrl = await generateBill(
      location,
      carNumber,
      ownerName,
      violatorName,
      speedData,
      challanAmount,
      minorDetails,
      phone_number,
      email,
      city,
      state,
      Challan_no,
      Payment_Status,
      Vehicle_name,
      Vehicle_type
    );

    await sendSMS(billUrl, phone_number);

    return {
      statusCode: 200,
      body: JSON.stringify({ bill_url: billUrl }),
    };
  } catch (error) {
    return {
      statusCode: 500,
      body: JSON.stringify({ error: error.message }),
    };
  }
};
