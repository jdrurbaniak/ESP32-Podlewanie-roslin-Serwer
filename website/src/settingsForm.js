const form = document.getElementById('deviceSettingsForm');

form.addEventListener('submit', async event => {
  event.preventDefault();

  const formData = new FormData(form);
  let formDataObject = Object.fromEntries(formData.entries());
  let formDataJsonString = JSON.stringify(formDataObject);

  let fetchOptions = {
    method: "POST",
    headers: {
      "Content-Type": "application/json",
      Accept: "application/json",
    },
    body: formDataJsonString,
  };
  let res = await fetch('/update-device-settings', fetchOptions);

  if (!res.ok) {
    let error = await res.text();
    throw new Error(error);
  }

  return res.json();
});
